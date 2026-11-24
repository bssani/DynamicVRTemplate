#include "Spectator/DvtSpectatorModel.h"
#include "Drive/DvtVehicle.h"
#include "Engine/World.h"
#include "EngineUtils.h"

bool DvtSpectator::FLayout::Update(FIntPoint WindowSize, float EyeAspect, float MinPanelWidth)
{
    if (WindowSize.X <= 0 || WindowSize.Y <= 0) { return false; }
    EyeAspect = FMath::IsFinite(EyeAspect) ? FMath::Max(0.01f,EyeAspect) : 4.f/3;
    MinPanelWidth = FMath::IsFinite(MinPanelWidth) ? FMath::Max(0.01f,MinPanelWidth) : 0.37f;
    if (WindowSize == LastSize && EyeAspect == LastAspect && MinPanelWidth == LastPanelWidth) { return false; }
    LastSize = WindowSize; LastAspect = EyeAspect; LastPanelWidth = MinPanelWidth;
    const double Aspect = static_cast<double>(WindowSize.X)/WindowSize.Y;
    const double Panel = FMath::Min(static_cast<double>(MinPanelWidth),Aspect*0.99);
    const double Height = FMath::Min(1.0,(Aspect-Panel)/EyeAspect);
    const double Width = Height*EyeAspect/Aspect;
    // Panel first avoids clearing the eye on the second copy (S5 D3).
    Layout = FSpectatorScreenModeTexturePlusEyeLayout(FVector2D(0,(1-Height)/2),FVector2D(Width,(1+Height)/2),
        FVector2D(Width,0),FVector2D(1,1),false,true,false);
    PanelPixels = FIntPoint(FMath::Max(1,FMath::RoundToInt((1-Width)*WindowSize.X)),WindowSize.Y);
    DrawScale = WindowSize.Y/1080.f;
    return true;
}

ADvtVehicle* DvtSpectator::FindVehicle(UWorld* World)
{
    if (!World) { return nullptr; }
    ADvtVehicle* Vehicle = nullptr;
    for (TActorIterator<ADvtVehicle> It(World); It; ++It)
    {
        if (Vehicle) { return nullptr; }
        Vehicle = *It;
    }
    return Vehicle && Vehicle->GetCarCount() >= 1 && Vehicle->GetCarIndex() >= 0 ? Vehicle : nullptr;
}

DvtSpectator::FPanel DvtSpectator::ReadPanel(const ADvtVehicle& Vehicle)
{
    FPanel Panel;
    Panel.Car = Vehicle.GetCarName();
    if (Panel.Car.IsEmpty())
    {
        Panel.Car = FText::Format(NSLOCTEXT("DvtSpectator","CarFallback","Car {0}"),FText::AsNumber(Vehicle.GetCarIndex()+1));
    }
    Panel.bShowHood = Vehicle.GetHoodCount() > 0;
    if (Panel.bShowHood)
    {
        Panel.Hood = Vehicle.GetHoodName();
        if (Panel.Hood.IsEmpty())
        {
            Panel.Hood = FText::Format(NSLOCTEXT("DvtSpectator","HoodFallback","Hood {0}"),FText::AsNumber(Vehicle.GetHoodIndex()+1));
        }
    }
    return Panel;
}
