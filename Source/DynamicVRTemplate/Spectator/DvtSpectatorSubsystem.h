#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Spectator/DvtSpectatorModel.h"
#include "UI/DvtUiStyle.h"
#include "DvtSpectatorSubsystem.generated.h"

class FWidgetRenderer;
class SWidget;
class UTextureRenderTarget2D;

UCLASS(Config=Game)
class DYNAMICVRTEMPLATE_API UDvtSpectatorSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()
public:
    UPROPERTY(Config, EditAnywhere, Category="Spectator", meta=(ClampMin="0.01"))
    float EyeAspect = 4.f/3;
    UPROPERTY(Config, EditAnywhere, Category="Spectator", meta=(ClampMin="0.01"))
    float MinPanelWidth = 0.37f;
    UPROPERTY(Config, EditAnywhere, Category="Spectator", meta=(ClampMin="1"))
    float RedrawsPerSecond = 10;
    UPROPERTY(Config, EditAnywhere, Category="Spectator|Style", meta=(ClampMin="1"))
    int32 HeadingFontSize = 16;
    UPROPERTY(Config, EditAnywhere, Category="Spectator|Style", meta=(ClampMin="1"))
    int32 CarFontSize = 40;
    UPROPERTY(Config, EditAnywhere, Category="Spectator|Style", meta=(ClampMin="1"))
    int32 HoodFontSize = 36;
    UPROPERTY(Config, EditAnywhere, Category="Spectator|Style")
    FLinearColor BackgroundColor = DvtUiStyle::Background;
    UPROPERTY(Config, EditAnywhere, Category="Spectator|Style")
    FLinearColor TextColor = DvtUiStyle::Text;
    UPROPERTY(Config, EditAnywhere, Category="Spectator|Style")
    FLinearColor SecondaryTextColor = DvtUiStyle::SecondaryText;
    UPROPERTY(Config, EditAnywhere, Category="Spectator|Style")
    FLinearColor LineColor = DvtUiStyle::Line;

    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return bPending || WidgetRenderer != nullptr; }
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UDvtSpectatorSubsystem, STATGROUP_Tickables); }
    virtual void Deinitialize() override;
protected:
    virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
private:
    friend struct FDvtSpectatorTestAccess;
    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> RenderTarget;
    UPROPERTY(Transient)
    TWeakObjectPtr<ADvtVehicle> Vehicle;
    FWidgetRenderer* WidgetRenderer = nullptr;
    TSharedPtr<SWidget> SlateWidget;
    DvtSpectator::FLayout ScreenLayout;
    DvtSpectator::FPanel CurrentPanel;
    bool bPending = false;
    float PendingSeconds = 0, RetryCountdown = 0, RedrawCountdown = 0;

    bool TryStart();
    void Stop();
    TSharedRef<SWidget> MakeWidget();
    TSharedRef<SWidget> MakeNameWidget(bool bCar);
};
