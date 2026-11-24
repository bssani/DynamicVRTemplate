#include "DvtOperatorToolbar.h"
#include "Operator/DvtOperatorModel.h"
#include "Operator/DvtOperatorSubsystem.h"
#include "UI/DvtUiStyle.h"
#include "Brushes/SlateImageBrush.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Framework/Commands/UIAction.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "ToolMenu.h"
#include "ToolMenus.h"

DvtOperatorToolbar::FState DvtOperatorToolbar::Evaluate(UWorld* PlayWorld)
{
    if (!PlayWorld || !PlayWorld->IsGameWorld()) { return {nullptr, FText::FromString(TEXT("Start Play first."))}; }
    if (!DvtOperator::FindController(PlayWorld))
    {
        return {nullptr, FText::FromString(TEXT("This level has no drive controller."))};
    }
    return {PlayWorld->GetSubsystem<UDvtOperatorSubsystem>(),
        FText::FromString(TEXT("Show the operator window. Restores it if minimized, reopens it if closed."))};
}

void DvtOperatorToolbar::Execute(UWorld* PlayWorld, TFunctionRef<void(UDvtOperatorSubsystem&)> Show)
{
    if (auto* Subsystem = Evaluate(PlayWorld).Subsystem.Get()) { Show(*Subsystem); }
}

void FDvtEditorToolsModule::StartupModule()
{
    if (bStarted) { return; }
    bStarted = true;
    Style = MakeShared<FSlateStyleSet>(DvtOperatorToolbar::StyleName);
    Style->SetContentRoot(FPaths::Combine(FPaths::ProjectDir(),TEXT("Resources/Editor")));
    Style->Set(DvtOperatorToolbar::BrushName, new FSlateVectorImageBrush(
        Style->RootToContentDir(TEXT("OperatorWindow.svg")),FVector2D(20,20),DvtUiStyle::Accent));
    FSlateStyleRegistry::RegisterSlateStyle(*Style);
    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this,&FDvtEditorToolsModule::RegisterMenus));
}

void FDvtEditorToolsModule::ShutdownModule()
{
    if (!bStarted) { return; }
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    FSlateStyleRegistry::UnRegisterSlateStyle(*Style);
    Style.Reset(); bStarted = false;
}

void FDvtEditorToolsModule::RegisterMenus()
{
    if (!bStarted) { return; }
    UToolMenus::UnregisterOwner(this);
    FToolMenuOwnerScoped Owner(this);
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(DvtOperatorToolbar::MenuName);
    auto& Section = Menu->FindOrAddSection(DvtOperatorToolbar::SectionName);
    const auto PlayWorld = []() -> UWorld* { return GEditor ? GEditor->PlayWorld.Get() : nullptr; };
    Section.AddEntry(FToolMenuEntry::InitToolBarButton(DvtOperatorToolbar::EntryName,
        FUIAction(FExecuteAction::CreateLambda([PlayWorld]()
            { DvtOperatorToolbar::Execute(PlayWorld(),[](UDvtOperatorSubsystem& S) { S.ShowOperatorWindow(); }); }),
            FCanExecuteAction::CreateLambda([PlayWorld]() { return DvtOperatorToolbar::Evaluate(PlayWorld()).CanExecute(); })),
        FText::FromString(TEXT("Operator")),
        TAttribute<FText>::CreateLambda([PlayWorld]() { return DvtOperatorToolbar::Evaluate(PlayWorld()).ToolTip; }),
        FSlateIcon(DvtOperatorToolbar::StyleName,DvtOperatorToolbar::BrushName)));
}

IMPLEMENT_MODULE(FDvtEditorToolsModule, DynamicVRTemplateEditorTools)
