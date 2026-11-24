#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class UDvtOperatorSubsystem;
class FSlateStyleSet;
class UWorld;

namespace DvtOperatorToolbar
{
    inline const FName StyleName(TEXT("DvtOperatorStyle")), BrushName(TEXT("DvtOperator.Icon"));
    inline const FName MenuName(TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar"));
    inline const FName SectionName(TEXT("DvtOperator")), EntryName(TEXT("DvtOperatorWindow"));
    struct FState
    {
        TWeakObjectPtr<UDvtOperatorSubsystem> Subsystem;
        FText ToolTip;
        bool CanExecute() const { return Subsystem.IsValid(); }
    };
    FState Evaluate(UWorld* PlayWorld);
    void Execute(UWorld* PlayWorld, TFunctionRef<void(UDvtOperatorSubsystem&)> Show);
}

class FDvtEditorToolsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    void RegisterMenus();
private:
    bool bStarted = false;
    TSharedPtr<FSlateStyleSet> Style;
};
