#pragma once

#include "CoreMinimal.h"
#include "UI/DvtCommandDecision.h"

class ADvtPlayerController;
class UWorld;

namespace DvtOperator
{
    enum class ECommand { PreviousCar, NextCar, SelectCar, PreviousHood, NextHood, SelectHood, Reapply, Reset };
    enum class EStyle { Primary, Secondary, Disabled, Current };
    struct FButton
    {
        ECommand Command;
        int32 Index = -1;
    };
    struct FStatus
    {
        FString Car, Hood, Alignment, LeftHand, RightHand;
        bool bApplied = false, bNeedsCheck = false;
    };
    struct FPlacement
    {
        bool bRemembered = false;
        FVector2D Position = FVector2D::ZeroVector, ClientSize = FVector2D(560, 860);
        void Remember(FVector2D InPosition, FVector2D InSize) { Position = InPosition; ClientSize = InSize; bRemembered = true; }
        static FPlacement& Session();
    };
    DYNAMICVRTEMPLATE_API ADvtPlayerController* FindController(UWorld* World);
    FDvtCommandDecision Evaluate(const ADvtPlayerController* Controller, FButton Button);
    FDvtCommandDecision CanReapply(bool bPresent, bool bFinding);
    void Execute(ADvtPlayerController* Controller, FButton Button);
    EStyle Style(const ADvtPlayerController* Controller, FButton Button);
    FStatus ReadStatus(const ADvtPlayerController* Controller);
    FString Name(const FText& Value, const TCHAR* Kind, int32 Index);
    FString AlignmentText(bool bPresent, bool bApplied, bool bNeedsCheck, const FString& Reason, bool bFinding);
    FString HandText(bool bAvailable, bool bTracked, bool bGripped);
    const TCHAR* Label(ECommand Command);
    const TCHAR* Key(ECommand Command);
}
