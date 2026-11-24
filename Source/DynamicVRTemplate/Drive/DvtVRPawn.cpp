#include "Drive/DvtVRPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

ADvtVRPawn::ADvtVRPawn()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("StageOrigin"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->bLockToHmd = true;
    SetActorEnableCollision(false);
}
