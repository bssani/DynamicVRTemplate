#include "Drive/DvtPlayerController.h"
#include "Drive/DvtResetPoint.h"
#include "Drive/DvtVehicle.h"
#include "Drive/DvtVRPawn.h"
#include "Calibration/DvtCalibrationLogic.h"
#include "VRCalAlignmentComponent.h"
#include "Grip/DvtSteeringGripComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogDvtDrive, Log, All);

ADvtPlayerController::ADvtPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

ADvtVehicle* ADvtPlayerController::GetDriveVehicle() const { return Vehicle.Get(); }
FDvtCommandDecision ADvtPlayerController::CanResetVehicle() const
{
    return IsValid(Vehicle) ? ResetSequence.CanRequest() : FDvtCommandDecision{false, TEXT("No drive vehicle")};
}

void ADvtPlayerController::BeginPlay()
{
    Super::BeginPlay();
    TArray<ADvtVehicle*> Vehicles;
    for (TActorIterator<ADvtVehicle> It(GetWorld()); It; ++It) { Vehicles.Add(*It); }
    if (!DvtDrive::HasSingleVehicle(Vehicles.Num()))
    {
        UE_LOG(LogDvtDrive, Error, TEXT("Expected exactly one drive vehicle; found %d. Driving disabled."), Vehicles.Num());
        return;
    }
    Vehicle = Vehicles[0];
    AttachVRPawn();
    ConnectTrackedHands();
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (auto* Inputs = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(); Inputs && DriveMappingContext)
        {
            Inputs->AddMappingContext(DriveMappingContext, 0);
        }
    }
}

void ADvtPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    AttachVRPawn();
    ConnectTrackedHands();
}

void ADvtPlayerController::ConnectTrackedHands()
{
    // Independent of attachment; calibration can own the pawn transform (S2 D7).
    if (const auto* VRPawn = Cast<ADvtVRPawn>(GetPawn()); IsValid(Vehicle) && VRPawn)
    {
        Vehicle->SteeringGrip->SetTrackedHands(VRPawn->LeftTrackedHand, VRPawn->RightTrackedHand);
    }
}

void ADvtPlayerController::AttachVRPawn()
{
    if (IsValid(Vehicle) && GetPawn() && DvtCalibration::ShouldAttachToDriver(GetPawn()->GetAttachParentActor(), Vehicle))
    {
        // Preserve the kit's relative transform in either startup order (S4 D3).
        GetPawn()->AttachToComponent(Vehicle->DriverOrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
}

void ADvtPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (auto* Inputs = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (SteerAction)
        {
            Inputs->BindAction(SteerAction, ETriggerEvent::Triggered, this, &ADvtPlayerController::Steer);
            Inputs->BindAction(SteerAction, ETriggerEvent::Completed, this, &ADvtPlayerController::SteerCompleted);
        }
        if (ThrottleAction)
        {
            Inputs->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ADvtPlayerController::Throttle);
            Inputs->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ADvtPlayerController::ThrottleCompleted);
        }
        if (BrakeAction)
        {
            Inputs->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ADvtPlayerController::Brake);
            Inputs->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ADvtPlayerController::BrakeCompleted);
        }
        if (ResetAction) { Inputs->BindAction(ResetAction, ETriggerEvent::Started, this, &ADvtPlayerController::ResetVehicle); }
        if (ReapplyAlignmentAction) { Inputs->BindAction(ReapplyAlignmentAction, ETriggerEvent::Started, this, &ADvtPlayerController::ReapplyAlignment); }
        if (PreviousCarAction) { Inputs->BindAction(PreviousCarAction, ETriggerEvent::Started, this, &ADvtPlayerController::PreviousCar); }
        if (NextCarAction) { Inputs->BindAction(NextCarAction, ETriggerEvent::Started, this, &ADvtPlayerController::NextCar); }
        if (PreviousHoodAction) { Inputs->BindAction(PreviousHoodAction, ETriggerEvent::Started, this, &ADvtPlayerController::PreviousHood); }
        if (NextHoodAction) { Inputs->BindAction(NextHoodAction, ETriggerEvent::Started, this, &ADvtPlayerController::NextHood); }
    }
}

void ADvtPlayerController::Steer(const FInputActionValue& Value)
{
    if (IsValid(Vehicle)) { Vehicle->ApplySteering(DvtDrive::Steering(Value.Get<float>())); }
}

void ADvtPlayerController::ReapplyAlignment()
{
    if (!IsValid(Vehicle) || !IsValid(Vehicle->Alignment)) { return; }
    bool bRequested;
#if WITH_DEV_AUTOMATION_TESTS
    if (ReapplyForTest) { bRequested = ReapplyForTest(Vehicle->Alignment); }
    else
#endif
    { bRequested = Vehicle->Alignment->RequestReapply(); }
    if (!bRequested)
    {
        UE_LOG(LogDvtDrive, Log, TEXT("Alignment reapply deferred while finding an anchor."));
    }
}

void ADvtPlayerController::Throttle(const FInputActionValue& Value)
{
    if (IsValid(Vehicle)) { Vehicle->GetVehicleMovementComponent()->SetThrottleInput(DvtDrive::Pedal(Value.Get<float>())); }
}

void ADvtPlayerController::Brake(const FInputActionValue& Value)
{
    if (IsValid(Vehicle)) { Vehicle->GetVehicleMovementComponent()->SetBrakeInput(DvtDrive::Pedal(Value.Get<float>())); }
}

void ADvtPlayerController::SteerCompleted(const FInputActionValue&) { Steer(FInputActionValue(0.f)); }
void ADvtPlayerController::ThrottleCompleted(const FInputActionValue&) { Throttle(FInputActionValue(0.f)); }
void ADvtPlayerController::BrakeCompleted(const FInputActionValue&) { Brake(FInputActionValue(0.f)); }

void ADvtPlayerController::PreviousCar() { if (IsValid(Vehicle)) { Vehicle->PreviousCar(); } }
void ADvtPlayerController::SelectCar(int32 Index) { if (IsValid(Vehicle)) { Vehicle->SelectCar(Index); } }
void ADvtPlayerController::SelectHood(int32 Index) { if (IsValid(Vehicle)) { Vehicle->SelectHood(Index); } }
void ADvtPlayerController::NextCar() { if (IsValid(Vehicle)) { Vehicle->NextCar(); } }
void ADvtPlayerController::PreviousHood() { if (IsValid(Vehicle)) { Vehicle->PreviousHood(); } }
void ADvtPlayerController::NextHood() { if (IsValid(Vehicle)) { Vehicle->NextHood(); } }

void ADvtPlayerController::ResetVehicle()
{
    if (!IsValid(Vehicle) || !ResetSequence.Request(ResetFadeOutSeconds, ResetFadeInSeconds)) { return; }
    TArray<FTransform> Points;
    for (TActorIterator<ADvtResetPoint> It(GetWorld()); It; ++It) { Points.Add(It->GetActorTransform()); }
    PendingResetTarget = DvtDrive::ResetTarget(Vehicle->GetActorLocation(), Points, Vehicle->GetStartTransform(), ResetLiftCm);
    if (PlayerCameraManager)
    {
        PlayerCameraManager->StartCameraFade(0, 1, FMath::Max(0.f, ResetFadeOutSeconds), FLinearColor::Black, false, true);
    }
    AdvanceReset(0);
}

void ADvtPlayerController::AdvanceReset(float DeltaSeconds)
{
    if (ResetSequence.Advance(DeltaSeconds))
    {
        if (IsValid(Vehicle)) { Vehicle->TeleportForReset(PendingResetTarget); }
        if (PlayerCameraManager)
        {
            PlayerCameraManager->StartCameraFade(1, 0, FMath::Max(0.f, ResetFadeInSeconds), FLinearColor::Black, false, false);
        }
    }
}

void ADvtPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    AdvanceReset(DeltaSeconds);
}
