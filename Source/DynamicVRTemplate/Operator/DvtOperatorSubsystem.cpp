#include "Operator/DvtOperatorSubsystem.h"
#include "Operator/SDvtOperatorPanel.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/App.h"
#include "Widgets/SWindow.h"

bool UDvtOperatorSubsystem::DoesSupportWorldType(EWorldType::Type Type) const
{
    return Type == EWorldType::Game || Type == EWorldType::PIE;
}

bool UDvtOperatorSubsystem::CanCreateWindow()
{
    return FApp::CanEverRender() && !GIsAutomationTesting && FSlateApplication::IsInitialized();
}

void UDvtOperatorSubsystem::Tick(float)
{
    if (Window) { RememberWindow(Window.ToSharedRef()); }
    if (!bOpened) { ShowOperatorWindow(); }
}

TSharedRef<SWindow> UDvtOperatorSubsystem::MakeWindow()
{
    const auto& Placement = DvtOperator::FPlacement::Session();
    return SNew(SWindow).Title(FText::FromString(TEXT("DynamicVRTemplate Operator")))
        .ClientSize(Placement.ClientSize).ScreenPosition(Placement.Position)
        .AdjustInitialSizeAndPositionForDPIScale(!Placement.bRemembered)
        .AutoCenter(Placement.bRemembered ? EAutoCenter::None : EAutoCenter::PreferredWorkArea)
        .HasCloseButton(false).SupportsMinimize(true).SupportsMaximize(false)
        .FocusWhenFirstShown(false).ActivationPolicy(EWindowActivationPolicy::Never)
        [SNew(SDvtOperatorPanel).Subsystem(this)];
}

void UDvtOperatorSubsystem::ShowOperatorWindow()
{
    if (!CanCreateWindow() || !DvtOperator::FindController(GetWorld())) { return; }
    if (!Window)
    {
        Window = MakeWindow();
        Window->SetOnWindowClosed(FOnWindowClosed::CreateWeakLambda(this, [this](const TSharedRef<SWindow>& ClosedWindow)
        { RememberWindow(ClosedWindow); Window.Reset(); }));
        FSlateApplication::Get().AddWindow(Window.ToSharedRef());
        bOpened = true;
    }
    if (Window->IsWindowMinimized()) { Window->Restore(); }
    Window->BringToFront(false);
    RestoreGameFocus();
}

void UDvtOperatorSubsystem::PressButton(DvtOperator::FButton Button)
{
    DvtOperator::Execute(DvtOperator::FindController(GetWorld()), Button);
    RestoreGameFocus();
}

void UDvtOperatorSubsystem::RestoreGameFocus()
{
#if WITH_DEV_AUTOMATION_TESTS
    if (RestoreFocusForTest) { RestoreFocusForTest(); return; }
#endif
    if (!CanCreateWindow()) { return; }
    FSlateApplication::Get().SetAllUserFocusToGameViewport();
    if (UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
    {
        if (auto GameWindow = Viewport->GetWindow()) { GameWindow->BringToFront(true); }
    }
}

void UDvtOperatorSubsystem::RememberWindow(const TSharedRef<SWindow>& ClosedWindow)
{
    // Iconic Windows dimensions must not replace the last usable placement (S6 D1).
    if (!ClosedWindow->IsWindowMinimized())
    {
        DvtOperator::FPlacement::Session().Remember(ClosedWindow->GetPositionInScreen(), ClosedWindow->GetClientSizeInScreen());
    }
}

void UDvtOperatorSubsystem::Deinitialize()
{
    if (Window)
    {
        const auto Closing = Window.ToSharedRef();
        RememberWindow(Closing);
        Closing->SetOnWindowClosed(FOnWindowClosed());
        Window.Reset();
        Closing->RequestDestroyWindow();
    }
    Super::Deinitialize();
}
