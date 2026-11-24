#include "Drive/DvtVehicle.h"
#include "Drive/DvtDriveLogic.h"
#include "Grip/DvtSteeringGripComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

ADvtVehicle::ADvtVehicle()
{
    AutoPossessPlayer = EAutoReceiveInput::Disabled;
    AutoPossessAI = EAutoPossessAI::Disabled;
    GetVehicleMovementComponent()->SetRequiresControllerForInputs(false);
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
}

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
