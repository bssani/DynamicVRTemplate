#pragma once

#include "CoreMinimal.h"
#include "Operator/DvtOperatorModel.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SCompoundWidget.h"

class UDvtOperatorSubsystem;
class SButton;
class SVerticalBox;
class SScrollBox;

namespace DvtOperator
{
    FButtonStyle ButtonStyle(EStyle Style);
    FLinearColor Foreground(EStyle Style);
    FSlateFontInfo Font(float Pixels, bool bBold = false);
}

class SDvtOperatorPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDvtOperatorPanel) {}
        SLATE_ARGUMENT(TWeakObjectPtr<UDvtOperatorSubsystem>, Subsystem)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    virtual void Tick(const FGeometry& Geometry, double CurrentTime, float DeltaTime) override;
private:
    friend struct FDvtOperatorTestAccess;
    TSharedRef<SWidget> Button(DvtOperator::FButton Command, const FString& Name = FString());
    TSharedRef<SWidget> Section(const TCHAR* Title, TSharedRef<SWidget> Content);
    TSharedRef<SWidget> StatusText(FString DvtOperator::FStatus::*Field, float Pixels, bool bBold = false, bool bHeader = false);
    void RebuildLists();
    TWeakObjectPtr<UDvtOperatorSubsystem> Subsystem;
    DvtOperator::FStatus Status;
    FButtonStyle Styles[4];
    FSlateBrush PanelBrush, HeaderBrush, WindowBrush, DotBrush;
    TSharedPtr<SVerticalBox> CarList, HoodList;
    TSharedPtr<SScrollBox> Scroll;
    struct FEntry { DvtOperator::FButton Command; TSharedPtr<SButton> Widget; };
    TArray<FEntry> Buttons;
    int32 LastCar = -2, LastCarCount = -1, LastHoodCount = -1;
};
