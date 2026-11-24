#if WITH_DEV_AUTOMATION_TESTS
#include "DvtOperatorToolbar.h"
#include "Operator/DvtOperatorSubsystem.h"
#include "Drive/DvtPlayerController.h"
#include "Drive/DvtVehicle.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateStyleRegistry.h"
#include "ToolMenu.h"
#include "ToolMenus.h"

namespace
{
    int32 EntryCount()
    {
        int32 Count = 0;
        if (const auto* Menu = UToolMenus::Get()->FindMenu(DvtOperatorToolbar::MenuName))
        {
            for (const auto& Section : Menu->Sections) for (const auto& Entry : Section.Blocks)
            { if (Entry.Name == DvtOperatorToolbar::EntryName) { ++Count; } }
        }
        return Count;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtToolbarState,"DynamicVRTemplate.S6.E14.ToolbarState",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtToolbarState::RunTest(const FString&)
{
    using namespace DvtOperatorToolbar;
    TestFalse(TEXT("No Play disabled"),Evaluate(nullptr).CanExecute());
    TestEqual(TEXT("No Play tooltip"),Evaluate(nullptr).ToolTip.ToString(),FString(TEXT("Start Play first.")));
    const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true)
        .ShouldSimulatePhysics(false).CreateNavigation(false).CreateAISystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game,false,NAME_None,nullptr,true,ERHIFeatureLevel::Num,&Values);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());
    TestEqual(TEXT("No drive controller tooltip"),Evaluate(World).ToolTip.ToString(),FString(TEXT("This level has no drive controller.")));
    auto* EngineController = World->SpawnActor<APlayerController>();
    TestFalse(TEXT("Calibration level disabled"),Evaluate(World).CanExecute());
    World->DestroyActor(EngineController);
    auto* Controller = World->SpawnActor<ADvtPlayerController>();
    TestFalse(TEXT("Unconnected controller disabled"),Evaluate(World).CanExecute());
    World->SpawnActor<ADvtVehicle>(); Controller->DispatchBeginPlay();
    TestTrue(TEXT("Drive level enabled"),Evaluate(World).CanExecute());
    int32 Calls = 0;
    Execute(World,[&](UDvtOperatorSubsystem& S)
    {
        TestTrue(TEXT("Uses this Play world's subsystem"),&S == World->GetSubsystem<UDvtOperatorSubsystem>());
        S.ShowOperatorWindow(); ++Calls;
    });
    TestEqual(TEXT("One Show call"),Calls,1);
    Execute(nullptr,[&](UDvtOperatorSubsystem&) { ++Calls; });
    TestEqual(TEXT("Disabled does not dispatch"),Calls,1);
    World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtToolbarRegistration,"DynamicVRTemplate.S6.E15.ToolbarRegistration",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtToolbarRegistration::RunTest(const FString&)
{
    using namespace DvtOperatorToolbar;
    auto& Module = FModuleManager::GetModuleChecked<FDvtEditorToolsModule>(TEXT("DynamicVRTemplateEditorTools"));
    Module.RegisterMenus(); Module.RegisterMenus(); Module.StartupModule();
    TestEqual(TEXT("One entry after repeat registration"),EntryCount(),1);
    const ISlateStyle* Style = FSlateStyleRegistry::FindSlateStyle(StyleName);
    if (!TestNotNull(TEXT("Style registered"),Style)) { return false; }
    const auto* Brush = Style->GetOptionalBrush(BrushName,nullptr,nullptr);
    if (!TestNotNull(TEXT("Icon registered"),Brush)) { return false; }
    const FString Path = FPaths::Combine(FPaths::ProjectDir(),TEXT("Resources/Editor/OperatorWindow.svg"));
    TestTrue(TEXT("SVG exists"),IFileManager::Get().FileExists(*Path));
    TestEqual(TEXT("SVG resource"),Brush->GetResourceName(),FName(*Path));
    TestTrue(TEXT("Vector brush 20 px"),Brush->ImageType == ESlateBrushImageType::Vector && Brush->ImageSize == FVector2f(20,20));
    TestTrue(TEXT("Blue brush tint"),Brush->TintColor.GetSpecifiedColor().ToFColorSRGB() == FColor(0x5b,0x8c,0xff));
    Module.ShutdownModule(); Module.ShutdownModule();
    TestEqual(TEXT("Entry removed on shutdown"),EntryCount(),0);
    TestNull(TEXT("Style removed on shutdown"),FSlateStyleRegistry::FindSlateStyle(StyleName));
    Module.StartupModule(); Module.RegisterMenus();
    TestEqual(TEXT("One entry after restart"),EntryCount(),1);
    TestNotNull(TEXT("Style restored"),FSlateStyleRegistry::FindSlateStyle(StyleName));
    return true;
}
#endif
