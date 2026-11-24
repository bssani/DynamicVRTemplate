#include "Drive/DvtVRPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "OculusXRHandComponent.h"

ADvtVRPawn::ADvtVRPawn()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("StageOrigin"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->bLockToHmd = true;
    LeftTrackedHand = CreateDefaultSubobject<UOculusXRHandComponent>(TEXT("LeftTrackedHand"));
    RightTrackedHand = CreateDefaultSubobject<UOculusXRHandComponent>(TEXT("RightTrackedHand"));
    LeftTrackedHand->SkeletonType = EOculusXRHandType::HandLeft;
    LeftTrackedHand->MeshType = EOculusXRHandType::HandLeft;
    RightTrackedHand->SkeletonType = EOculusXRHandType::HandRight;
    RightTrackedHand->MeshType = EOculusXRHandType::HandRight;
    for (auto* Hand : {LeftTrackedHand.Get(), RightTrackedHand.Get()})
    {
        Hand->SetupAttachment(RootComponent);
        // SteeringGrip owns visibility (S2 D3).
        Hand->ConfidenceBehavior = EOculusXRConfidenceBehavior::None;
        Hand->bInitializePhysics = false;
        Hand->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Hand->SetVisibility(false, true);
    }
    SetActorEnableCollision(false);
}
