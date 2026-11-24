#pragma once

#include "CoreMinimal.h"
#include "HeadMountedDisplayTypes.h"

class ADvtVehicle;
class UWorld;

namespace DvtSpectator
{
    struct FLayout
    {
        FSpectatorScreenModeTexturePlusEyeLayout Layout;
        FIntPoint PanelPixels = FIntPoint::ZeroValue;
        float DrawScale = 1;
        bool Update(FIntPoint WindowSize, float EyeAspect, float MinPanelWidth);
        FVector2D DesignSize() const { return FVector2D(PanelPixels)/DrawScale; }
    private:
        FIntPoint LastSize = FIntPoint::ZeroValue;
        float LastAspect = 0, LastPanelWidth = 0;
    };

    struct FPanel
    {
        FText Car, Hood;
        bool bShowHood = false;
    };

    ADvtVehicle* FindVehicle(UWorld* World);
    FPanel ReadPanel(const ADvtVehicle& Vehicle);
}
