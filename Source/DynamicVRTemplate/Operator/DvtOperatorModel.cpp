#include "Operator/DvtOperatorModel.h"
#include "Drive/DvtPlayerController.h"
#include "Drive/DvtVehicle.h"
#include "Grip/DvtSteeringGripComponent.h"
#include "VRCalAlignmentComponent.h"
#include "Engine/World.h"

using namespace DvtOperator;

FPlacement& FPlacement::Session() { static FPlacement Placement; return Placement; }

ADvtPlayerController* DvtOperator::FindController(UWorld* World)
{
    auto* Controller = World && World->IsGameWorld() ? Cast<ADvtPlayerController>(World->GetFirstPlayerController()) : nullptr;
    return Controller && IsValid(Controller->GetDriveVehicle()) ? Controller : nullptr;
}

FDvtCommandDecision DvtOperator::Evaluate(const ADvtPlayerController* Controller, FButton Button)
{
    const ADvtVehicle* Vehicle = IsValid(Controller) ? Controller->GetDriveVehicle() : nullptr;
    if (!IsValid(Vehicle)) { return {false, TEXT("No drive vehicle")}; }
    switch (Button.Command)
    {
    case ECommand::PreviousCar: case ECommand::NextCar: return Vehicle->CanStepCar();
    case ECommand::PreviousHood: case ECommand::NextHood: return Vehicle->CanStepHood();
    case ECommand::SelectCar: return Vehicle->CanSelectCar(Button.Index);
    case ECommand::SelectHood: return Vehicle->CanSelectHood(Button.Index);
    case ECommand::Reset: return Controller->CanResetVehicle();
    case ECommand::Reapply:
        return CanReapply(IsValid(Vehicle->Alignment), IsValid(Vehicle->Alignment) && Vehicle->Alignment->IsFindingAnchor());
    }
    return {false, TEXT("Unknown command")};
}

FDvtCommandDecision DvtOperator::CanReapply(bool bPresent, bool bFinding)
{
    if (!bPresent) { return {false, TEXT("No alignment in this level")}; }
    return bFinding ? FDvtCommandDecision{false, TEXT("Finding anchor\u2026")} : FDvtCommandDecision{};
}

void DvtOperator::Execute(ADvtPlayerController* Controller, FButton Button)
{
    if (!IsValid(Controller)) { return; }
    // Dispatch to the exact functions bound to keys; commands own acceptance (S6 D3).
    switch (Button.Command)
    {
    case ECommand::PreviousCar: Controller->PreviousCar(); break;
    case ECommand::NextCar: Controller->NextCar(); break;
    case ECommand::SelectCar: Controller->SelectCar(Button.Index); break;
    case ECommand::PreviousHood: Controller->PreviousHood(); break;
    case ECommand::NextHood: Controller->NextHood(); break;
    case ECommand::SelectHood: Controller->SelectHood(Button.Index); break;
    case ECommand::Reapply: Controller->ReapplyAlignment(); break;
    case ECommand::Reset: Controller->ResetVehicle(); break;
    }
}

EStyle DvtOperator::Style(const ADvtPlayerController* Controller, FButton Button)
{
    const FDvtCommandDecision Decision = Evaluate(Controller, Button);
    if (!Decision) { return Decision.Reason == TEXT("Current") ? EStyle::Current : EStyle::Disabled; }
    const auto* Vehicle = Controller->GetDriveVehicle();
    return Button.Command == ECommand::Reapply && Vehicle->Alignment->State == EVRCalAlignmentState::NeedsCheck
        ? EStyle::Primary : EStyle::Secondary;
}

FString DvtOperator::Name(const FText& Value, const TCHAR* Kind, int32 Index)
{
    return Value.IsEmpty() ? FString::Printf(TEXT("%s %d"), Kind, Index + 1) : Value.ToString();
}

FString DvtOperator::AlignmentText(bool bPresent, bool bApplied, bool bNeedsCheck, const FString& Reason, bool bFinding)
{
    if (!bPresent) { return TEXT("No alignment in this level"); }
    FString Text = bApplied ? TEXT("Applied") : bNeedsCheck ? TEXT("Needs check \u2014 ") + Reason : TEXT("No saved alignment");
    if (bFinding) { Text += TEXT(" \u00b7 Finding anchor\u2026"); }
    return Text;
}

FString DvtOperator::HandText(bool bAvailable, bool bTracked, bool bGripped)
{
    return FString(bAvailable ? bTracked ? TEXT("Tracked") : TEXT("Not tracked") : TEXT("Hand tracking off"))
        + TEXT(" \u00b7 ") + (bGripped ? TEXT("On wheel") : TEXT("Off wheel"));
}

FStatus DvtOperator::ReadStatus(const ADvtPlayerController* Controller)
{
    FStatus Status;
    const auto* Vehicle = IsValid(Controller) ? Controller->GetDriveVehicle() : nullptr;
    if (!IsValid(Vehicle)) { return Status; }
    Status.Car = Vehicle->GetCarCount() == 0 ? TEXT("No cars in this vehicle") : FString::Printf(TEXT("%s (%d/%d)"),
        *Name(Vehicle->GetCarName(), TEXT("Car"), Vehicle->GetCarIndex()), Vehicle->GetCarIndex()+1, Vehicle->GetCarCount());
    Status.Hood = Vehicle->GetHoodCount() == 0 ? TEXT("No hoods") : FString::Printf(TEXT("%s (%d/%d)"),
        *Name(Vehicle->GetHoodName(), TEXT("Hood"), Vehicle->GetHoodIndex()), Vehicle->GetHoodIndex()+1, Vehicle->GetHoodCount());
    const auto* Alignment = Vehicle->Alignment.Get();
    Status.bApplied = IsValid(Alignment) && Alignment->State == EVRCalAlignmentState::Applied;
    Status.bNeedsCheck = IsValid(Alignment) && Alignment->State == EVRCalAlignmentState::NeedsCheck;
    Status.Alignment = AlignmentText(IsValid(Alignment), Status.bApplied, Status.bNeedsCheck,
        IsValid(Alignment) ? Alignment->Reason : FString(), IsValid(Alignment) && Alignment->IsFindingAnchor());
    const auto* Grip = Vehicle->SteeringGrip.Get();
    const auto Hand = [Grip](EControllerHand Side)
    {
        return HandText(IsValid(Grip) && Grip->IsHandTrackingAvailable(), IsValid(Grip) && Grip->IsHandTracked(Side),
            IsValid(Grip) && Grip->GetGrip(Side) == EDvtHandGrip::Gripped);
    };
    Status.LeftHand = Hand(EControllerHand::Left); Status.RightHand = Hand(EControllerHand::Right);
    return Status;
}

const TCHAR* DvtOperator::Label(ECommand Command)
{
    switch (Command)
    {
    case ECommand::PreviousCar: return TEXT("Previous car");
    case ECommand::NextCar: return TEXT("Next car");
    case ECommand::PreviousHood: return TEXT("Previous hood");
    case ECommand::NextHood: return TEXT("Next hood");
    case ECommand::Reapply: return TEXT("Reapply alignment");
    case ECommand::Reset: return TEXT("Reset vehicle");
    default: return TEXT("");
    }
}

const TCHAR* DvtOperator::Key(ECommand Command)
{
    switch (Command)
    {
    case ECommand::PreviousCar: return TEXT("PageUp");
    case ECommand::NextCar: return TEXT("PageDown");
    case ECommand::PreviousHood: return TEXT("[");
    case ECommand::NextHood: return TEXT("]");
    case ECommand::Reapply: return TEXT("F9");
    case ECommand::Reset: return TEXT("Backspace");
    default: return TEXT("");
    }
}
