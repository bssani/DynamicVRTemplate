#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Operator/DvtOperatorModel.h"
#include "Operator/DvtOperatorSubsystem.h"
#include "Operator/SDvtOperatorPanel.h"
#include "Drive/DvtPlayerController.h"
#include "Drive/DvtVehicle.h"
#include "Cars/DvtCarBody.h"
#include "Cars/DvtHoodVariantComponent.h"
#include "Grip/DvtSteeringGripComponent.h"
#include "UI/DvtUiStyle.h"
#include "VRCalAlignmentComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SWindow.h"

using namespace DvtOperator;

struct FDvtOperatorTestAccess
{
    static void Link(ADvtPlayerController* C, ADvtVehicle* V) { C->Vehicle = V; }
    static DvtDrive::EResetPhase Phase(const ADvtPlayerController* C) { return C->ResetSequence.Phase; }
    static void Reset(ADvtPlayerController* C) { C->ResetSequence = {}; }
    static void Reapply(ADvtPlayerController* C, TFunction<bool(UVRCalAlignmentComponent*)> Fn) { C->ReapplyForTest = MoveTemp(Fn); }
    static void Focus(UDvtOperatorSubsystem* S, TFunction<void()> Fn) { S->RestoreFocusForTest = MoveTemp(Fn); }
    static bool HasWindow(UDvtOperatorSubsystem* S) { return S->Window.IsValid(); }
    static TSharedRef<SWindow> MakeWindow(UDvtOperatorSubsystem* S) { return S->MakeWindow(); }
    static void Sample(UDvtSteeringGripComponent* G, EControllerHand H, bool Available, bool Tracked, float Distance)
    { G->ApplyHandSample(H,Available,Tracked,Distance); }
    static bool ButtonsUnfocusable(const TSharedRef<SDvtOperatorPanel>& Panel)
    {
        for (const auto& B : Panel->Buttons) { if (B.Widget->SupportsKeyboardFocus()) { return false; } }
        return !Panel->Scroll->SupportsKeyboardFocus();
    }
    static TSharedRef<SWidget> LongStatus(const TSharedRef<SDvtOperatorPanel>& Panel)
    {
        Panel->Status.Alignment = FString::ChrN(200,TEXT('W'));
        return Panel->StatusText(&FStatus::Alignment,14);
    }
};

namespace
{
    struct FOperatorWorld
    {
        UWorld* World;
        ADvtVehicle* Vehicle;
        ADvtPlayerController* Controller;
        UDvtOperatorSubsystem* Subsystem;
        FOperatorWorld()
        {
            const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true)
                .ShouldSimulatePhysics(false).CreateNavigation(false).CreateAISystem(false);
            World = UWorld::CreateWorld(EWorldType::Game,false,NAME_None,nullptr,true,ERHIFeatureLevel::Num,&Values);
            GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
            World->InitializeActorsForPlay(FURL());
            Vehicle = World->SpawnActor<ADvtVehicle>();
            for (int32 Count : {3,2,0})
            {
                auto* Car = World->SpawnActor<ADvtCarBody>();
                Car->AttachToActor(Vehicle,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
                for (int32 I = 0; I < Count; ++I)
                {
                    auto* Hood = NewObject<UDvtHoodVariantComponent>(Car);
                    Car->AddInstanceComponent(Hood); Hood->SetupAttachment(Car->GetRootComponent());
                    Hood->SortOrder = I; Hood->RegisterComponent();
                }
                Vehicle->Cars.Add(Car);
            }
            Vehicle->DispatchBeginPlay();
            Controller = World->SpawnActor<ADvtPlayerController>();
            FDvtOperatorTestAccess::Link(Controller,Vehicle);
            Subsystem = World->GetSubsystem<UDvtOperatorSubsystem>();
        }
        ~FOperatorWorld()
        {
            World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
        }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtOperatorGates,"DynamicVRTemplate.S6.E1E8E9E10.Window",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtOperatorGates::RunTest(const FString&)
{
    FOperatorWorld F;
    TestTrue(TEXT("Drive controller with vehicle"),FindController(F.World) == F.Controller);
    TestFalse(TEXT("Automation cannot create a native window"),UDvtOperatorSubsystem::CanCreateWindow());
    F.Subsystem->ShowOperatorWindow(); F.Subsystem->Tick(0.1f);
    TestFalse(TEXT("Explicit show and tick stay windowless"),FDvtOperatorTestAccess::HasWindow(F.Subsystem));
    FDvtOperatorTestAccess::Link(F.Controller,nullptr);
    TestNull(TEXT("No vehicle cannot show"),FindController(F.World));
    F.World->DestroyActor(F.Controller);
    F.World->SpawnActor<APlayerController>();
    TestNull(TEXT("Calibration controller cannot show"),FindController(F.World));

    if (!FSlateApplication::IsInitialized()) { AddError(TEXT("Slate required")); return false; }
    const FPlacement Previous = FPlacement::Session();
    FPlacement::Session() = {};
    auto First = FDvtOperatorTestAccess::MakeWindow(F.Subsystem);
    TestTrue(TEXT("Never activates"),First->ActivationPolicy() == EWindowActivationPolicy::Never);
    TestFalse(TEXT("No initial focus"),First->IsFocusedInitially());
    TestTrue(TEXT("Default client size"),FVector2D(First->GetClientSizeInScreen()).Equals(FVector2D(560,860)));
    FPlacement::Session().Remember(FVector2D(120,150),FVector2D(600,700));
    auto Next = FDvtOperatorTestAccess::MakeWindow(F.Subsystem);
    TestTrue(TEXT("Remembered client size"),FVector2D(Next->GetClientSizeInScreen()).Equals(FVector2D(600,700)));
    TestTrue(TEXT("Remembered creation position"),FVector2D(Next->GetInitialDesiredPositionInScreen()).Equals(FVector2D(120,150)));
    FPlacement::Session() = Previous;
    auto Panel = SNew(SDvtOperatorPanel).Subsystem(F.Subsystem);
    TestFalse(TEXT("Panel takes no keyboard focus"),Panel->SupportsKeyboardFocus());
    TestTrue(TEXT("Buttons and scroll do not take focus"),FDvtOperatorTestAccess::ButtonsUnfocusable(Panel));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtOperatorPredicates,"DynamicVRTemplate.S6.E3E4.Predicates",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtOperatorPredicates::RunTest(const FString&)
{
    for (int32 Cars : {0,1,3}) for (int32 Hoods : {0,1,2}) for (double Now : {0.0,0.4,0.5})
    {
        DvtSelection::FSelection S; TArray<int32> Counts; Counts.Init(Hoods,Cars); S.Initialize(Counts,0);
        S.LastCarTime = S.LastHoodTime = 0;
        for (int32 I = -1; I <= Cars; ++I)
        {
            auto Copy = S;
            TestEqual(TEXT("Car select gate matches command"),S.CanSelectCar(I).bAccepted,Copy.SelectCar(I,Now));
        }
        for (int32 I = -1; I <= Hoods; ++I)
        {
            auto Copy = S;
            TestEqual(TEXT("Hood select gate matches command"),S.CanSelectHood(I).bAccepted,Copy.SelectHood(I,Now));
        }
        for (int32 Direction : {-1,1})
        {
            auto Copy = S;
            TestEqual(TEXT("Car step gate matches command"),S.CanStepCar(Now,0.5).bAccepted,Copy.StepCar(Direction,Now,0.5));
            Copy = S;
            TestEqual(TEXT("Hood step gate matches command"),S.CanStepHood(Now,0.5).bAccepted,Copy.StepHood(Direction,Now,0.5));
        }
    }
    DvtSelection::FSelection S; S.Initialize({2,2,2},0);
    S.StepCar(1,0,0.5);
    TestEqual(TEXT("Cooldown tooltip"),S.CanStepCar(0.4,0.5).Reason,FString(TEXT("Switching\u2026")));
    TestTrue(TEXT("Absolute selection available during cooldown"),S.CanSelectCar(2).bAccepted);
    TestTrue(TEXT("Exact boundary enabled"),S.CanStepCar(0.5,0.5).bAccepted);
    TestEqual(TEXT("Current tooltip"),S.CanSelectCar(1).Reason,FString(TEXT("Current")));
    S.Initialize({1},0);
    TestEqual(TEXT("Single car tooltip"),S.CanStepCar(0,0.5).Reason,FString(TEXT("Only one car")));
    TestEqual(TEXT("Single hood tooltip"),S.CanStepHood(0,0.5).Reason,FString(TEXT("Only one hood")));
    S.Initialize({0},0);
    TestEqual(TEXT("No hood tooltip"),S.CanStepHood(0,0.5).Reason,FString(TEXT("No hoods")));
    S.Initialize({},0);
    TestEqual(TEXT("No cars tooltip"),S.CanStepCar(0,0.5).Reason,FString(TEXT("No cars in this vehicle")));
    DvtDrive::FResetSequence Reset;
    const bool IdleGate = Reset.CanRequest().bAccepted;
    TestEqual(TEXT("Reset idle gate matches"),IdleGate,Reset.Request(0.3f,0.4f));
    const bool ActiveGate = Reset.CanRequest().bAccepted;
    TestEqual(TEXT("Reset active gate matches"),ActiveGate,Reset.Request(0.3f,0.4f));
    TestEqual(TEXT("Reset tooltip"),Reset.CanRequest().Reason,FString(TEXT("Reset in progress")));
    TestTrue(TEXT("Reapply idle"),CanReapply(true,false).bAccepted);
    TestEqual(TEXT("Missing component tooltip"),CanReapply(false,false).Reason,FString(TEXT("No alignment in this level")));
    TestEqual(TEXT("Finding tooltip"),CanReapply(true,true).Reason,FString(TEXT("Finding anchor\u2026")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtOperatorDispatch,"DynamicVRTemplate.S6.E2E5E8.Dispatch",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtOperatorDispatch::RunTest(const FString&)
{
    const ECommand Commands[] = {ECommand::PreviousCar,ECommand::NextCar,ECommand::PreviousHood,ECommand::NextHood,ECommand::Reset,ECommand::Reapply};
    for (ECommand Command : Commands)
    {
        FOperatorWorld KeyWorld, ButtonWorld;
        auto* C = KeyWorld.Controller;
        C->PreviousCarAction = NewObject<UInputAction>(C); C->NextCarAction = NewObject<UInputAction>(C);
        C->PreviousHoodAction = NewObject<UInputAction>(C); C->NextHoodAction = NewObject<UInputAction>(C);
        C->ResetAction = NewObject<UInputAction>(C); C->ReapplyAlignmentAction = NewObject<UInputAction>(C);
        UInputAction* Action = Command == ECommand::PreviousCar ? C->PreviousCarAction.Get() : Command == ECommand::NextCar ? C->NextCarAction.Get()
            : Command == ECommand::PreviousHood ? C->PreviousHoodAction.Get() : Command == ECommand::NextHood ? C->NextHoodAction.Get()
            : Command == ECommand::Reset ? C->ResetAction.Get() : C->ReapplyAlignmentAction.Get();
        auto* Inputs = NewObject<UEnhancedInputComponent>(C); C->InputComponent = Inputs; C->SetupInputComponent();
        int32 KeyReapply = 0, ButtonReapply = 0, Focus = 0, Bindings = 0;
        FDvtOperatorTestAccess::Reapply(C,[&](UVRCalAlignmentComponent*) { ++KeyReapply; return true; });
        FDvtOperatorTestAccess::Reapply(ButtonWorld.Controller,[&](UVRCalAlignmentComponent*) { ++ButtonReapply; return true; });
        FDvtOperatorTestAccess::Focus(ButtonWorld.Subsystem,[&]() { ++Focus; });
        for (const auto& Binding : Inputs->GetActionEventBindings())
        {
            if (Binding->GetAction() == Action)
            {
                TestTrue(TEXT("Key binds Started"),Binding->GetTriggerEvent() == ETriggerEvent::Started);
                Binding->Execute(FInputActionInstance(Action)); ++Bindings;
            }
        }
        ButtonWorld.Subsystem->PressButton({Command});
        TestEqual(TEXT("Exactly one key binding"),Bindings,1);
        TestEqual(TEXT("Same resulting car"),ButtonWorld.Vehicle->GetCarIndex(),KeyWorld.Vehicle->GetCarIndex());
        TestEqual(TEXT("Same resulting hood"),ButtonWorld.Vehicle->GetHoodIndex(),KeyWorld.Vehicle->GetHoodIndex());
        TestEqual(TEXT("Same car cooldown"),ButtonWorld.Vehicle->CanStepCar().bAccepted,KeyWorld.Vehicle->CanStepCar().bAccepted);
        TestEqual(TEXT("Same hood cooldown"),ButtonWorld.Vehicle->CanStepHood().bAccepted,KeyWorld.Vehicle->CanStepHood().bAccepted);
        TestTrue(TEXT("Same reset phase"),FDvtOperatorTestAccess::Phase(C) == FDvtOperatorTestAccess::Phase(ButtonWorld.Controller));
        TestEqual(TEXT("Same reapply calls"),ButtonReapply,KeyReapply);
        TestEqual(TEXT("Focus restored once"),Focus,1);
        if (Command == ECommand::Reapply) { TestEqual(TEXT("One reapply command"),ButtonReapply,1); }
    }
    FOperatorWorld F;
    int32 Focus = 0; FDvtOperatorTestAccess::Focus(F.Subsystem,[&]() { ++Focus; });
    F.Subsystem->PressButton({ECommand::SelectHood,2});
    F.Subsystem->PressButton({ECommand::SelectCar,1});
    TestEqual(TEXT("Clamped hood"),F.Vehicle->GetHoodIndex(),1);
    F.Subsystem->PressButton({ECommand::SelectCar,2});
    TestEqual(TEXT("Third car selected"),F.Vehicle->GetCarIndex(),2);
    TestTrue(TEXT("Current selection style"),Style(F.Controller,{ECommand::SelectCar,2}) == EStyle::Current);
    F.Subsystem->PressButton({ECommand::SelectCar,0});
    TestEqual(TEXT("Requested hood restored"),F.Vehicle->GetHoodIndex(),2);
    F.Subsystem->PressButton({ECommand::SelectCar,0});
    TestEqual(TEXT("Rejected current command still restores focus"),Focus,5);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtOperatorStatus,"DynamicVRTemplate.S6.E6E7.Status",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtOperatorStatus::RunTest(const FString&)
{
    FOperatorWorld F;
    auto Status = ReadStatus(F.Controller);
    TestEqual(TEXT("Empty car name fallback and count"),Status.Car,FString(TEXT("Car 1 (1/3)")));
    TestEqual(TEXT("Empty hood name fallback and count"),Status.Hood,FString(TEXT("Hood 1 (1/3)")));
    F.Vehicle->Cars[0]->DisplayName = FText::FromString(TEXT("Sample"));
    TestEqual(TEXT("Authored car name"),ReadStatus(F.Controller).Car,FString(TEXT("Sample (1/3)")));
    F.Controller->SelectCar(2);
    TestEqual(TEXT("No hoods state"),ReadStatus(F.Controller).Hood,FString(TEXT("No hoods")));
    TestEqual(TEXT("Applied state"),AlignmentText(true,true,false,TEXT(""),false),FString(TEXT("Applied")));
    TestEqual(TEXT("Reason unchanged with anchor suffix"),AlignmentText(true,false,true,TEXT("Check origin"),true),FString(TEXT("Needs check \u2014 Check origin \u00b7 Finding anchor\u2026")));
    TestEqual(TEXT("No saved state"),AlignmentText(true,false,false,TEXT(""),false),FString(TEXT("No saved alignment")));
    TestEqual(TEXT("No component state"),AlignmentText(false,false,false,TEXT(""),false),FString(TEXT("No alignment in this level")));
    auto* G = F.Vehicle->SteeringGrip.Get();
    FDvtOperatorTestAccess::Sample(G,EControllerHand::Left,false,false,-1);
    TestEqual(TEXT("Tracking off"),ReadStatus(F.Controller).LeftHand,FString(TEXT("Hand tracking off \u00b7 On wheel")));
    FDvtOperatorTestAccess::Sample(G,EControllerHand::Left,true,true,25);
    FDvtOperatorTestAccess::Sample(G,EControllerHand::Right,true,false,-1);
    TestEqual(TEXT("Left tracked released"),ReadStatus(F.Controller).LeftHand,FString(TEXT("Tracked \u00b7 Off wheel")));
    TestEqual(TEXT("Right tracking lost retains grip"),ReadStatus(F.Controller).RightHand,FString(TEXT("Not tracked \u00b7 On wheel")));
    TestTrue(TEXT("Sample availability cached"),G->IsHandTrackingAvailable());
    TestTrue(TEXT("Left sample cached"),G->IsHandTracked(EControllerHand::Left));
    TestFalse(TEXT("Right sample cached"),G->IsHandTracked(EControllerHand::Right));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtOperatorDesign,"DynamicVRTemplate.S6.E11E12E13.Design",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtOperatorDesign::RunTest(const FString&)
{
    FOperatorWorld F;
    const ECommand Commands[] = {ECommand::PreviousCar,ECommand::NextCar,ECommand::PreviousHood,ECommand::NextHood,ECommand::SelectCar,ECommand::SelectHood,ECommand::Reset,ECommand::Reapply};
    for (EVRCalAlignmentState State : {EVRCalAlignmentState::Applied,EVRCalAlignmentState::NeedsCheck,EVRCalAlignmentState::NoSavedValue})
    {
        F.Vehicle->Alignment->State = State;
        int32 Primaries = 0;
        for (ECommand Command : Commands)
        {
            if (Style(F.Controller,{Command,0}) == EStyle::Primary)
            { ++Primaries; TestTrue(TEXT("Only reapply can be primary"),Command == ECommand::Reapply); }
        }
        TestEqual(TEXT("At most one primary, needs check only"),Primaries,State == EVRCalAlignmentState::NeedsCheck ? 1 : 0);
    }
    TestTrue(TEXT("Regular secondary"),Style(F.Controller,{ECommand::NextCar}) == EStyle::Secondary);
    F.Controller->NextCar();
    TestTrue(TEXT("Cooldown disabled style"),Style(F.Controller,{ECommand::NextCar}) == EStyle::Disabled);
    const FLinearColor Colors[] = {DvtUiStyle::Window,DvtUiStyle::Background,DvtUiStyle::Selected,DvtUiStyle::Line,DvtUiStyle::ButtonOutline,DvtUiStyle::Text,DvtUiStyle::SecondaryText,DvtUiStyle::Accent,DvtUiStyle::Header,DvtUiStyle::Applied,DvtUiStyle::NeedsCheck};
    const TCHAR* Hex[] = {TEXT("0b0f17"),TEXT("131a26"),TEXT("1c2534"),TEXT("2e3a4e"),TEXT("66748a"),TEXT("eef2f7"),TEXT("9aa6b8"),TEXT("5b8cff"),TEXT("0d2a66"),TEXT("3cc285"),TEXT("f2b544")};
    for (int32 I = 0; I < UE_ARRAY_COUNT(Colors); ++I)
    { TestTrue(TEXT("Exact palette sRGB"),Colors[I].ToFColorSRGB() == FColor::FromHex(Hex[I])); }
    for (EStyle Kind : {EStyle::Primary,EStyle::Secondary,EStyle::Disabled,EStyle::Current})
    {
        const auto Button = ButtonStyle(Kind);
        TestTrue(TEXT("Rounded corners 6 px"),Button.Normal.OutlineSettings.CornerRadii == FVector4(6,6,6,6));
        TestEqual(TEXT("Outline 1 px"),Button.Normal.OutlineSettings.Width,1.f);
        TestTrue(TEXT("Disabled drawing uses same brush"),Button.Disabled.TintColor == Button.Normal.TintColor);
    }
    const auto Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    for (ECommand Command : Commands)
    {
        const double Width = Measure->Measure(Label(Command),Font(14)).X + Measure->Measure(Key(Command),Font(12)).X + 32;
        TestTrue(TEXT("Button name plus key fits default panel"),Width < 232);
    }
    for (const TCHAR* Text : {TEXT("Hand tracking off \u00b7 On wheel"),TEXT("Not tracked \u00b7 Off wheel"),TEXT("No alignment in this level")})
    { TestTrue(TEXT("Status fits one line"),Measure->Measure(Text,Font(14)).X < 400); }
    TestTrue(TEXT("Long name requires ellipsis"),Measure->Measure(FString::ChrN(50,TEXT('W')),Font(20,true)).X > 456);
    auto Panel = SNew(SDvtOperatorPanel).Subsystem(F.Subsystem);
    auto LongText = FDvtOperatorTestAccess::LongStatus(Panel);
    auto Constrained = SNew(SBox).WidthOverride(456).HeightOverride(24)[LongText];
    Constrained->SlatePrepass(1.f);
    TestEqual(TEXT("Long status stays in constrained row"),Constrained->GetDesiredSize().X,456.f);
    return true;
}
#endif
