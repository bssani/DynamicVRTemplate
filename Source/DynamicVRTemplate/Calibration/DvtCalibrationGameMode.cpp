#include "Calibration/DvtCalibrationGameMode.h"
#include "GameFramework/PlayerController.h"

ADvtCalibrationGameMode::ADvtCalibrationGameMode()
{
    // Reserve S for saving calibration (S4 D5).
    PlayerControllerClass = APlayerController::StaticClass();
}
