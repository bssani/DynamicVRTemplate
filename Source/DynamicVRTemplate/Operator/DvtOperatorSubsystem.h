#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Operator/DvtOperatorModel.h"
#include "DvtOperatorSubsystem.generated.h"

class SWindow;

UCLASS()
class DYNAMICVRTEMPLATE_API UDvtOperatorSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UDvtOperatorSubsystem, STATGROUP_Tickables); }
    virtual bool IsTickable() const override { return !IsTemplate(); }
    void ShowOperatorWindow();
    void PressButton(DvtOperator::FButton Button);
    void RestoreGameFocus();
    static bool CanCreateWindow();
protected:
    virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
private:
    friend struct FDvtOperatorTestAccess;
    TSharedRef<SWindow> MakeWindow();
    void RememberWindow(const TSharedRef<SWindow>& ClosedWindow);
    TSharedPtr<SWindow> Window;
    bool bOpened = false;
#if WITH_DEV_AUTOMATION_TESTS
    TFunction<void()> RestoreFocusForTest;
#endif
};
