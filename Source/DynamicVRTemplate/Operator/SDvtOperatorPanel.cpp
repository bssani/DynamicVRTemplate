#include "Operator/SDvtOperatorPanel.h"
#include "Operator/DvtOperatorSubsystem.h"
#include "Drive/DvtPlayerController.h"
#include "Drive/DvtVehicle.h"
#include "UI/DvtUiStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"

using namespace DvtOperator;

FSlateFontInfo DvtOperator::Font(float Pixels, bool bBold)
{
    return FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Pixels * 0.75f);
}

FLinearColor DvtOperator::Foreground(EStyle Style)
{
    return Style == EStyle::Primary ? DvtUiStyle::Window : Style == EStyle::Disabled ? DvtUiStyle::ButtonOutline : DvtUiStyle::Text;
}

FButtonStyle DvtOperator::ButtonStyle(EStyle Style)
{
    const auto Fill = Style == EStyle::Primary ? DvtUiStyle::Accent : Style == EStyle::Disabled ? DvtUiStyle::Window
        : Style == EStyle::Current ? DvtUiStyle::Selected : DvtUiStyle::Background;
    const auto Outline = Style == EStyle::Primary || Style == EStyle::Current ? DvtUiStyle::Accent
        : Style == EStyle::Disabled ? DvtUiStyle::Line : DvtUiStyle::ButtonOutline;
    const FSlateRoundedBoxBrush Brush(Fill, 6.f, Outline, 1.f);
    return FButtonStyle().SetNormal(Brush).SetHovered(Brush).SetPressed(Brush).SetDisabled(Brush)
        .SetNormalPadding(FMargin(12,0)).SetPressedPadding(FMargin(12,0));
}

TSharedRef<SWidget> SDvtOperatorPanel::StatusText(FString FStatus::*Field, float Pixels, bool bBold, bool bHeader)
{
    return SNew(STextBlock).Font(Font(Pixels, bBold)).ColorAndOpacity(bHeader ? FLinearColor::White : DvtUiStyle::Text)
        .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
        .Text_Lambda([this, Field]() { return FText::FromString(Status.*Field); })
        .ToolTipText_Lambda([this, Field]() { return FText::FromString(Status.*Field); });
}

TSharedRef<SWidget> SDvtOperatorPanel::Button(FButton Command, const FString& Name)
{
    const auto Controller = [this]() { return Subsystem.IsValid() ? FindController(Subsystem->GetWorld()) : nullptr; };
    const auto ButtonKind = [Controller, Command]() { return Style(Controller(), Command); };
    TSharedPtr<SButton> Widget;
    const auto Result = SNew(SBox).HeightOverride(40)
    [
        SAssignNew(Widget, SButton).IsFocusable(false).ButtonStyle(&Styles[static_cast<int32>(ButtonKind())])
        .IsEnabled_Lambda([Controller, Command]() { return Evaluate(Controller(), Command).bAccepted; })
        .ToolTipText_Lambda([Controller, Command, Name]()
        {
            const auto Decision = Evaluate(Controller(), Command);
            const FString Title = Name.IsEmpty() ? Label(Command.Command) : Name;
            return FText::FromString(Decision.Reason.IsEmpty() ? Title : Title + TEXT(" \u2014 ") + Decision.Reason);
        })
        .OnClicked_Lambda([this, Command]()
        {
            if (Subsystem.IsValid()) { Subsystem->PressButton(Command); }
            return FReply::Handled();
        })
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
            [SNew(STextBlock).Text(FText::FromString(Name.IsEmpty() ? Label(Command.Command) : Name))
                .Font(Font(14)).OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                .ColorAndOpacity_Lambda([ButtonKind]() { return FSlateColor(Foreground(ButtonKind())); })]
            + SHorizontalBox::Slot().AutoWidth().Padding(8,0,0,0).VAlign(VAlign_Center)
            [SNew(STextBlock).Font(Font(12))
                .Text_Lambda([ButtonKind, Command]() { return FText::FromString(ButtonKind() == EStyle::Current ? TEXT("Current") : Key(Command.Command)); })
                .ColorAndOpacity_Lambda([ButtonKind]() { return FSlateColor(Foreground(ButtonKind())); })]
        ]
    ];
    Buttons.Add({Command, Widget});
    return Result;
}

TSharedRef<SWidget> SDvtOperatorPanel::Section(const TCHAR* Title, TSharedRef<SWidget> Content)
{
    return SNew(SBorder).BorderImage(&PanelBrush).Padding(16)
    [SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,10)
        [SNew(STextBlock).Text(FText::FromString(Title)).Font(Font(13,true)).ColorAndOpacity(DvtUiStyle::SecondaryText)]
        + SVerticalBox::Slot().AutoHeight()[Content]];
}

void SDvtOperatorPanel::RebuildLists()
{
    const auto* Controller = Subsystem.IsValid() ? FindController(Subsystem->GetWorld()) : nullptr;
    const auto* Vehicle = Controller ? Controller->GetDriveVehicle() : nullptr;
    if (!Vehicle) { return; }
    if (LastCar == Vehicle->GetCarIndex() && LastCarCount == Vehicle->GetCarCount() && LastHoodCount == Vehicle->GetHoodCount()) { return; }
    LastCar = Vehicle->GetCarIndex(); LastCarCount = Vehicle->GetCarCount(); LastHoodCount = Vehicle->GetHoodCount();
    Buttons.RemoveAll([](const FEntry& Entry) { return Entry.Command.Command == ECommand::SelectCar || Entry.Command.Command == ECommand::SelectHood; });
    CarList->ClearChildren(); HoodList->ClearChildren();
    for (int32 I = 0; I < LastCarCount; ++I)
    {
        CarList->AddSlot().AutoHeight().Padding(0,6,0,0)[Button({ECommand::SelectCar,I}, Name(Vehicle->GetCarNameAt(I),TEXT("Car"),I))];
    }
    for (int32 I = 0; I < LastHoodCount; ++I)
    {
        HoodList->AddSlot().AutoHeight().Padding(0,6,0,0)[Button({ECommand::SelectHood,I}, Name(Vehicle->GetHoodNameAt(I),TEXT("Hood"),I))];
    }
    if (!LastHoodCount && LastCarCount)
    {
        HoodList->AddSlot().AutoHeight().Padding(0,6,0,0)[SNew(STextBlock).Text(FText::FromString(TEXT("No hoods")))
            .Font(Font(14)).ColorAndOpacity(DvtUiStyle::SecondaryText)];
    }
}

void SDvtOperatorPanel::Construct(const FArguments& Args)
{
    Subsystem = Args._Subsystem;
    for (int32 I = 0; I < 4; ++I) { Styles[I] = ButtonStyle(static_cast<EStyle>(I)); }
    PanelBrush = FSlateRoundedBoxBrush(DvtUiStyle::Background, 6.f, DvtUiStyle::Line, 1.f);
    HeaderBrush = FSlateRoundedBoxBrush(DvtUiStyle::Header, 6.f);
    WindowBrush = FSlateRoundedBoxBrush(DvtUiStyle::Window, 0.f);
    DotBrush = FSlateRoundedBoxBrush(FLinearColor::White, 4.f);
    const auto Controller = Subsystem.IsValid() ? FindController(Subsystem->GetWorld()) : nullptr;
    Status = ReadStatus(Controller);
    SAssignNew(CarList,SVerticalBox); SAssignNew(HoodList,SVerticalBox);
    const auto Pair = [this](ECommand Left, ECommand Right)
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,4,0)[Button({Left})]
            + SHorizontalBox::Slot().FillWidth(1).Padding(4,0,0,0)[Button({Right})];
    };
    const auto CarContent = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[Pair(ECommand::PreviousCar,ECommand::NextCar)]
        + SVerticalBox::Slot().AutoHeight()[CarList.ToSharedRef()];
    const auto HoodContent = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[Pair(ECommand::PreviousHood,ECommand::NextHood)]
        + SVerticalBox::Slot().AutoHeight()[HoodList.ToSharedRef()];
    const auto EmptyCars = []()
    {
        return SNew(STextBlock).Text(FText::FromString(TEXT("No cars in this vehicle")))
            .Font(Font(14)).ColorAndOpacity(DvtUiStyle::SecondaryText);
    };
    const auto HasCars = [this]() { return LastCarCount > 0 ? EVisibility::Visible : EVisibility::Collapsed; };
    const auto NoCars = [this]() { return LastCarCount == 0 ? EVisibility::Visible : EVisibility::Collapsed; };
    CarContent->SetVisibility(TAttribute<EVisibility>::CreateLambda(HasCars));
    HoodContent->SetVisibility(TAttribute<EVisibility>::CreateLambda(HasCars));
    const auto CarEmpty = EmptyCars(); CarEmpty->SetVisibility(TAttribute<EVisibility>::CreateLambda(NoCars));
    const auto HoodEmpty = EmptyCars(); HoodEmpty->SetVisibility(TAttribute<EVisibility>::CreateLambda(NoCars));
    ChildSlot
    [SNew(SBorder).BorderImage(&WindowBrush).Padding(12)
        [SAssignNew(Scroll,SScrollBox).ScrollBarDragFocusCause(EFocusCause::SetDirectly)
            .OnUserScrolled_Lambda([this](float) { if (Subsystem.IsValid()) { Subsystem->RestoreGameFocus(); } })
            + SScrollBox::Slot().Padding(0,0,0,10)
            [SNew(SBorder).BorderImage(&HeaderBrush).Padding(16)
                [SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[StatusText(&FStatus::Car,20,true,true)]
                    + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[StatusText(&FStatus::Hood,16,false,true)]]]
            + SScrollBox::Slot().Padding(0,0,0,10)[Section(TEXT("Car"),SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[CarEmpty] + SVerticalBox::Slot().AutoHeight()[CarContent])]
            + SScrollBox::Slot().Padding(0,0,0,10)[Section(TEXT("Hood"),SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[HoodEmpty] + SVerticalBox::Slot().AutoHeight()[HoodContent])]
            + SScrollBox::Slot().Padding(0,0,0,10)[Section(TEXT("Alignment"),SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,10)
                [SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,8,0)
                    [SNew(SBox).WidthOverride(8).HeightOverride(8)
                        [SNew(SImage).Image(&DotBrush).ColorAndOpacity_Lambda([this]()
                        { return FSlateColor(Status.bApplied ? DvtUiStyle::Applied : DvtUiStyle::NeedsCheck); })]]
                    + SHorizontalBox::Slot().FillWidth(1)[StatusText(&FStatus::Alignment,14)]]
                + SVerticalBox::Slot().AutoHeight()[Button({ECommand::Reapply})])]
            + SScrollBox::Slot().Padding(0,0,0,10)[Section(TEXT("Vehicle"),Button({ECommand::Reset}))]
            + SScrollBox::Slot().Padding(0,0,0,10)[Section(TEXT("Hands"),SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(64)[SNew(STextBlock).Text(FText::FromString(TEXT("Left"))).Font(Font(14)).ColorAndOpacity(DvtUiStyle::SecondaryText)]]
                    + SHorizontalBox::Slot().FillWidth(1)[StatusText(&FStatus::LeftHand,14)]]
                + SVerticalBox::Slot().AutoHeight().Padding(0,6,0,0)
                [SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(64)[SNew(STextBlock).Text(FText::FromString(TEXT("Right"))).Font(Font(14)).ColorAndOpacity(DvtUiStyle::SecondaryText)]]
                    + SHorizontalBox::Slot().FillWidth(1)[StatusText(&FStatus::RightHand,14)]] )]
            + SScrollBox::Slot()[SNew(STextBlock).Text(FText::FromString(TEXT("Shortcut keys work in the game window.")))
                .Font(Font(12)).ColorAndOpacity(DvtUiStyle::SecondaryText)]
        ]
    ];
    Scroll->SetIsFocusable(false);
    RebuildLists();
}

void SDvtOperatorPanel::Tick(const FGeometry& Geometry, double CurrentTime, float DeltaTime)
{
    SCompoundWidget::Tick(Geometry,CurrentTime,DeltaTime);
    Status = ReadStatus(Subsystem.IsValid() ? FindController(Subsystem->GetWorld()) : nullptr);
    RebuildLists();
    for (auto& Entry : Buttons)
    {
        Entry.Widget->SetButtonStyle(&Styles[static_cast<int32>(Style(Subsystem.IsValid() ? FindController(Subsystem->GetWorld()) : nullptr, Entry.Command))]);
    }
}
