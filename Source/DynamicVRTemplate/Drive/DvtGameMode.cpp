#include "Drive/DvtGameMode.h"
#include "Drive/DvtPlayerController.h"
#include "Drive/DvtVRPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"

ADvtGameMode::ADvtGameMode()
{
    DefaultPawnClass = ADvtVRPawn::StaticClass();
    PlayerControllerClass = ADvtPlayerController::StaticClass();
}

void ADvtGameMode::StartPlay()
{
    UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Stage);
    UHeadMountedDisplayFunctionLibrary::SetWorldToMetersScale(this, 100);
    Super::StartPlay();
}
