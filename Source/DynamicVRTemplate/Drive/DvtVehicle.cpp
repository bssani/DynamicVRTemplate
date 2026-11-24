#include "Drive/DvtVehicle.h"
#include "Drive/DvtDriveLogic.h"
#include "Cars/DvtCarBody.h"
#include "Grip/DvtSteeringGripComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "VRCalAlignmentComponent.h"
#include "VRCalAlignPointComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogDvtSelection, Log, All);

ADvtVehicle::ADvtVehicle()
{
    AutoPossessPlayer = EAutoReceiveInput::Disabled;
    AutoPossessAI = EAutoPossessAI::Disabled;
    GetVehicleMovementComponent()->SetRequiresControllerForInputs(false);
    Alignment = CreateDefaultSubobject<UVRCalAlignmentComponent>(TEXT("Alignment"));
    AlignPoint1 = CreateDefaultSubobject<UVRCalAlignPointComponent>(TEXT("AlignPoint1"));
    AlignPoint2 = CreateDefaultSubobject<UVRCalAlignPointComponent>(TEXT("AlignPoint2"));
    AlignPoint3 = CreateDefaultSubobject<UVRCalAlignPointComponent>(TEXT("AlignPoint3"));
    AlignPoint4 = CreateDefaultSubobject<UVRCalAlignPointComponent>(TEXT("AlignPoint4"));
    int32 Order = 1;
    for (auto* Point : {AlignPoint1.Get(), AlignPoint2.Get(), AlignPoint3.Get(), AlignPoint4.Get()})
    {
        Point->SetupAttachment(GetRootComponent());
        Point->Order = Order++;
    }
    DriverOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("DriverOrigin"));
    DriverOrigin->SetupAttachment(GetMesh());
    SteeringWheelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("SteeringWheelPivot"));
    SteeringWheelPivot->SetupAttachment(GetMesh());
    SteeringWheelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SteeringWheelMesh"));
    SteeringWheelMesh->SetupAttachment(SteeringWheelPivot);
    SteeringWheelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SteeringGrip = CreateDefaultSubobject<UDvtSteeringGripComponent>(TEXT("SteeringGrip"));
    SteeringGrip->SetupAttachment(SteeringWheelPivot);
    LeftGripHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftGripHand"));
    LeftGripHand->SetupAttachment(SteeringWheelPivot);
    LeftGripHand->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightGripHand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightGripHand"));
    RightGripHand->SetupAttachment(SteeringWheelPivot);
    RightGripHand->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SteeringGrip->SetGripHands(LeftGripHand, RightGripHand);
}

void ADvtVehicle::BeginPlay()
{
    Super::BeginPlay();
    StartTransform = GetActorTransform();
    InitialWheelRotation = SteeringWheelPivot->GetRelativeRotation().Quaternion();
    InitializeCars();
}

void ADvtVehicle::InitializeCars()
{
    TArray<TObjectPtr<ADvtCarBody>> ValidCars;
    for (ADvtCarBody* Car : Cars)
    {
        if (!IsValid(Car)) { UE_LOG(LogDvtSelection, Warning, TEXT("Removed null car entry.")); }
        else if (ValidCars.Contains(Car)) { UE_LOG(LogDvtSelection, Warning, TEXT("Removed duplicate car entry: %s."), *Car->GetName()); }
        else { ValidCars.Add(Car); }
    }
    Cars = MoveTemp(ValidCars);
    TArray<int32> Counts;
    for (ADvtCarBody* Car : Cars)
    {
        Car->PrepareForSelection();
        Counts.Add(Car->GetHoodCount());
    }
    Selection.Initialize(Counts, InitialCar);
    if (Cars.IsEmpty()) { UE_LOG(LogDvtSelection, Warning, TEXT("No cars configured; keeping the sample chassis.")); }
    else
    {
        if (bHideChassisWhenCarsExist)
        {
            TInlineComponentArray<UPrimitiveComponent*> Components(this);
            for (UPrimitiveComponent* Component : Components)
            {
                if (Component == SteeringWheelPivot || Component->IsAttachedTo(SteeringWheelPivot)) { continue; }
                OriginalBodyVisibility.Add({Component, bool(Component->bHiddenInGame)});
                // Keep the wheel and hands independent of the shared chassis (S3 D8).
                Component->SetHiddenInGame(true);
            }
        }
        ApplySelection(true, false);
    }
}

void ADvtVehicle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (ADvtCarBody* Car : Cars) { if (IsValid(Car)) { Car->RestoreVisibility(); } }
    for (const FBodyVisibility& Saved : OriginalBodyVisibility)
    {
        if (Saved.Component.IsValid()) { Saved.Component->SetHiddenInGame(Saved.bHiddenInGame); }
    }
    OriginalBodyVisibility.Reset();
    Super::EndPlay(EndPlayReason);
}

ADvtCarBody* ADvtVehicle::GetSelectedCar() const
{
    return Cars.IsValidIndex(Selection.CarIndex) && IsValid(Cars[Selection.CarIndex]) ? Cars[Selection.CarIndex].Get() : nullptr;
}

FText ADvtVehicle::GetCarName() const
{
    const ADvtCarBody* Car = GetSelectedCar();
    return Car ? Car->DisplayName : FText::GetEmpty();
}

FText ADvtVehicle::GetHoodName() const
{
    const ADvtCarBody* Car = GetSelectedCar();
    return Car ? Car->GetHoodName(Selection.HoodIndex) : FText::GetEmpty();
}

double ADvtVehicle::ReadSwitchTimeSeconds() const { return GetWorld()->GetTimeSeconds(); }

void ADvtVehicle::ApplySelection(bool bCarChanged, bool bNotify)
{
    ADvtCarBody* Selected = GetSelectedCar();
    if (!Selected) { return; }
    if (bCarChanged)
    {
        for (ADvtCarBody* Car : Cars) { if (IsValid(Car)) { Car->SetActorHiddenInGame(Car != Selected); } }
        SteeringWheelMesh->SetStaticMesh(Selected->SteeringWheelMesh);
        SteeringWheelMesh->SetRelativeTransform(Selected->SteeringWheelMeshTransform);
        SteeringWheelMesh->SetVisibility(Selected->SteeringWheelMesh != nullptr);
        SteeringGrip->RimRadiusCm = Selected->RimRadiusCm;
        SteeringGrip->UpdateBounds();
        LeftGripHand->SetRelativeTransform(Selected->LeftGripHandTransform);
        RightGripHand->SetRelativeTransform(Selected->RightGripHandTransform);
    }
    Selected->ApplyHood(Selection.HoodIndex);
    if (bNotify)
    {
        UE_LOG(LogDvtSelection, Log, TEXT("Selected car %d, hood %d."), Selection.CarIndex, Selection.HoodIndex);
        OnSelectionChanged.Broadcast(Selection.CarIndex, Selection.HoodIndex);
    }
}

void ADvtVehicle::SelectCar(int32 Index)
{
    if (Cars.IsEmpty()) { return; }
    if (!Cars.IsValidIndex(Index))
    {
        UE_LOG(LogDvtSelection, Warning, TEXT("Invalid car index %d (count %d)."), Index, Cars.Num());
        return;
    }
    if (Selection.SelectCar(Index, ReadSwitchTimeSeconds())) { ApplySelection(true, true); }
}

void ADvtVehicle::SelectHood(int32 Index)
{
    if (Cars.IsEmpty()) { return; }
    if (Index < 0 || Index >= GetHoodCount())
    {
        UE_LOG(LogDvtSelection, Warning, TEXT("Invalid hood index %d (count %d)."), Index, GetHoodCount());
        return;
    }
    if (Selection.SelectHood(Index, ReadSwitchTimeSeconds())) { ApplySelection(false, true); }
}

void ADvtVehicle::StepCar(int32 Direction)
{
    if (Selection.StepCar(Direction, ReadSwitchTimeSeconds(), SwitchCooldownSeconds)) { ApplySelection(true, true); }
}

void ADvtVehicle::StepHood(int32 Direction)
{
    if (Selection.StepHood(Direction, ReadSwitchTimeSeconds(), SwitchCooldownSeconds)) { ApplySelection(false, true); }
}

void ADvtVehicle::NextCar() { StepCar(1); }
void ADvtVehicle::PreviousCar() { StepCar(-1); }
void ADvtVehicle::NextHood() { StepHood(1); }
void ADvtVehicle::PreviousHood() { StepHood(-1); }

void ADvtVehicle::ApplySteering(float Value)
{
    Value = DvtDrive::Steering(Value);
    GetVehicleMovementComponent()->SetSteeringInput(Value);
    FVector Axis = FVector::ForwardVector;
    if (SteeringSpinAxis == EAxis::Y) { Axis = FVector::RightVector; }
    else if (SteeringSpinAxis == EAxis::Z) { Axis = FVector::UpVector; }
    SteeringWheelPivot->SetRelativeRotation(InitialWheelRotation * FQuat(Axis,
        FMath::DegreesToRadians(DvtDrive::WheelAngle(Value, SteeringLockToLockDegrees))));
}

void ADvtVehicle::TeleportForReset(const FTransform& Target)
{
    SetActorLocationAndRotation(Target.GetLocation(), Target.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);
    GetMesh()->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
    GetMesh()->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    GetVehicleMovementComponent()->ResetVehicle();
    ApplySteering(0);
}
