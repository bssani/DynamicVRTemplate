#include "Spectator/DvtSpectatorSubsystem.h"
#include "Drive/DvtVehicle.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "IHeadMountedDisplay.h"
#include "ISpectatorScreenController.h"
#include "IXRTrackingSystem.h"
#include "Misc/App.h"
#include "Rendering/SlateRenderer.h"
#include "Slate/SceneViewport.h"
#include "Slate/WidgetRenderer.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    ISpectatorScreenController* SpectatorScreen(bool bRequireActive = true)
    {
        if (!FApp::CanEverRender() || !GEngine || !GEngine->XRSystem.IsValid()) { return nullptr; }
        IHeadMountedDisplay* HMD = GEngine->XRSystem->GetHMDDevice();
        return HMD && (!bRequireActive || (GEngine->IsStereoscopic3D() && HMD->IsHMDConnected() && HMD->IsHMDEnabled()))
            ? HMD->GetSpectatorScreenController() : nullptr;
    }

    FIntPoint WindowSize(UWorld* World)
    {
        UGameViewportClient* Client = World ? World->GetGameViewport() : nullptr;
        FSceneViewport* Viewport = Client ? Client->GetGameViewport() : nullptr;
        // VR viewport size may be the eye render size (S5 D3).
        const TSharedPtr<SWindow> Window = Viewport ? Viewport->FindWindow() : nullptr;
        if (!Window) { return FIntPoint::ZeroValue; }
        const FVector2D Size = Window->GetClientSizeInScreen();
        return FIntPoint(FMath::RoundToInt(Size.X),FMath::RoundToInt(Size.Y));
    }
}

bool UDvtSpectatorSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UDvtSpectatorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    bPending = FApp::CanEverRender();
    PendingSeconds = RetryCountdown = 0;
}

bool UDvtSpectatorSubsystem::TryStart()
{
    ISpectatorScreenController* Screen = SpectatorScreen();
    if (!Screen || !FSlateApplication::IsInitialized() || !GetWorld()->GetFirstPlayerController()) { return false; }
    Vehicle = DvtSpectator::FindVehicle(GetWorld());
    if (!Vehicle.IsValid()) { return false; }
    ScreenLayout = DvtSpectator::FLayout();
    if (!ScreenLayout.Update(WindowSize(GetWorld()),EyeAspect,MinPanelWidth)) { return false; }
    CurrentPanel = DvtSpectator::ReadPanel(*Vehicle.Get());
    SlateWidget = MakeWidget();
    WidgetRenderer = new FWidgetRenderer(true);
    RenderTarget = FWidgetRenderer::CreateTargetFor(FVector2D(ScreenLayout.PanelPixels),TF_Bilinear,false);
    if (!RenderTarget) { Stop(); return false; }
    RenderTarget->ClearColor = BackgroundColor;
    Screen->SetSpectatorScreenModeTexturePlusEyeLayout(ScreenLayout.Layout);
    Screen->SetSpectatorScreenTexture(RenderTarget);
    Screen->SetSpectatorScreenMode(ESpectatorScreenMode::TexturePlusEye);
    bPending = false;
    RedrawCountdown = 0;
    return true;
}

void UDvtSpectatorSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bPending)
    {
        PendingSeconds += DeltaTime;
        RetryCountdown -= DeltaTime;
        if (RetryCountdown <= 0)
        {
            RetryCountdown = 0.5f;
            TryStart();
        }
        if (bPending && PendingSeconds >= 30) { bPending = false; }
    }
    if (!RenderTarget) { return; }
    ISpectatorScreenController* Screen = SpectatorScreen();
    if (!Screen || !Vehicle.IsValid()) { Stop(); return; }
    RedrawCountdown -= DeltaTime;
    if (RedrawCountdown > 0) { return; }
    const float Interval = 1.f/FMath::Max(1.f,RedrawsPerSecond);
    RedrawCountdown = Interval;
    const FIntPoint Size = WindowSize(GetWorld());
    if (Size.X <= 0 || Size.Y <= 0) { return; }
    CurrentPanel = DvtSpectator::ReadPanel(*Vehicle.Get());
    if (ScreenLayout.Update(Size,EyeAspect,MinPanelWidth))
    {
        Screen->SetSpectatorScreenModeTexturePlusEyeLayout(ScreenLayout.Layout);
        if (RenderTarget->SizeX != ScreenLayout.PanelPixels.X || RenderTarget->SizeY != ScreenLayout.PanelPixels.Y)
        {
            RenderTarget->ResizeTarget(ScreenLayout.PanelPixels.X,ScreenLayout.PanelPixels.Y);
        }
    }
    WidgetRenderer->DrawWidget(RenderTarget,SlateWidget.ToSharedRef(),ScreenLayout.DrawScale,FVector2D(ScreenLayout.PanelPixels),Interval);
}

TSharedRef<SWidget> UDvtSpectatorSubsystem::MakeNameWidget(bool bCar)
{
    const auto Font = [this,bCar] { return FCoreStyle::GetDefaultFontStyle(bCar ? "Bold" : "Regular",bCar ? CarFontSize : HoodFontSize); };
    return SNew(SBox).MaxDesiredHeight_Lambda([Font]
    {
        const auto Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        return 3.f*Measure->GetMaxCharacterHeight(Font());
    })
    [
        SNew(STextBlock).Text_Lambda([this,bCar] { return bCar ? CurrentPanel.Car : CurrentPanel.Hood; })
        .Font_Lambda(Font).ColorAndOpacity_Lambda([this] { return FSlateColor(TextColor); })
        .WrapTextAt_Lambda([this] { return FMath::Max(1.0,ScreenLayout.DesignSize().X-66); })
        .WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
        .OverflowPolicy(ETextOverflowPolicy::Ellipsis).Clipping(EWidgetClipping::ClipToBounds)
    ];
}

TSharedRef<SWidget> UDvtSpectatorSubsystem::MakeWidget()
{
    const auto HeadingFont = [this]
    {
        FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Regular",HeadingFontSize);
        Font.LetterSpacing = 80;
        return Font;
    };
    return SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).Padding(0)
    .BorderBackgroundColor_Lambda([this] { return BackgroundColor; })
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()
        [
            SNew(SBox).WidthOverride(2)
            [
                SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).Padding(0)
                .BorderBackgroundColor_Lambda([this] { return LineColor; })
            ]
        ]
        + SHorizontalBox::Slot().FillWidth(1).Padding(32,40)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(STextBlock).Text(FText::FromString(TEXT("CAR"))).Font_Lambda(HeadingFont)
                .ColorAndOpacity_Lambda([this] { return FSlateColor(SecondaryTextColor); })
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)[MakeNameWidget(true)]
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SVerticalBox).Visibility_Lambda([this] { return CurrentPanel.bShowHood ? EVisibility::Visible : EVisibility::Collapsed; })
                + SVerticalBox::Slot().AutoHeight().Padding(0,32,0,32)
                [
                    SNew(SBox).HeightOverride(2)
                    [
                        SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).Padding(0)
                        .BorderBackgroundColor_Lambda([this] { return LineColor; })
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("HOOD"))).Font_Lambda(HeadingFont)
                    .ColorAndOpacity_Lambda([this] { return FSlateColor(SecondaryTextColor); })
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)[MakeNameWidget(false)]
            ]
        ]
    ];
}

void UDvtSpectatorSubsystem::Stop()
{
    if (RenderTarget)
    {
        if (ISpectatorScreenController* Screen = SpectatorScreen(false))
        {
            Screen->SetSpectatorScreenTexture(nullptr);
            Screen->SetSpectatorScreenMode(ESpectatorScreenMode::SingleEyeCroppedToFill);
        }
    }
    // Defer release until render-thread Slate work finishes (S5 D1).
    if (WidgetRenderer) { BeginCleanup(WidgetRenderer); WidgetRenderer = nullptr; }
    SlateWidget.Reset(); RenderTarget = nullptr; Vehicle.Reset();
    bPending = false;
}

void UDvtSpectatorSubsystem::Deinitialize()
{
    Stop();
    Super::Deinitialize();
}
