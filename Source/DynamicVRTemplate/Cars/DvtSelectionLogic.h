#pragma once

#include "CoreMinimal.h"
#include "UI/DvtCommandDecision.h"

namespace DvtSelection
{
    inline bool HoodLess(int32 OrderA, const FString& NameA, int32 OrderB, const FString& NameB)
    {
        return OrderA != OrderB ? OrderA < OrderB : NameA.Compare(NameB, ESearchCase::CaseSensitive) < 0;
    }

    struct FSelection
    {
        TArray<int32> HoodCounts;
        int32 CarIndex = -1;
        int32 HoodIndex = -1;
        int32 RequestedHoodIndex = 0;
        double LastCarTime = -TNumericLimits<double>::Max();
        double LastHoodTime = -TNumericLimits<double>::Max();

        void Initialize(const TArray<int32>& Counts, int32 InitialCar)
        {
            *this = FSelection();
            HoodCounts = Counts;
            CarIndex = Counts.IsEmpty() ? -1 : FMath::Clamp(InitialCar, 0, Counts.Num() - 1);
            ApplyRequestedHood();
        }

        int32 HoodCount() const { return HoodCounts.IsValidIndex(CarIndex) ? HoodCounts[CarIndex] : 0; }

        void ApplyRequestedHood()
        {
            HoodIndex = HoodCount() > 0 ? FMath::Min(RequestedHoodIndex, HoodCount() - 1) : -1;
        }

        FDvtCommandDecision CanSelectCar(int32 Index) const
        {
            if (HoodCounts.IsEmpty()) { return {false, TEXT("No cars in this vehicle")}; }
            if (!HoodCounts.IsValidIndex(Index)) { return {false, TEXT("Invalid car index")}; }
            return Index == CarIndex ? FDvtCommandDecision{false, TEXT("Current")} : FDvtCommandDecision{};
        }

        FDvtCommandDecision CanSelectHood(int32 Index) const
        {
            if (HoodCount() == 0) { return {false, TEXT("No hoods")}; }
            if (Index < 0 || Index >= HoodCount()) { return {false, TEXT("Invalid hood index")}; }
            return Index == HoodIndex ? FDvtCommandDecision{false, TEXT("Current")} : FDvtCommandDecision{};
        }

        FDvtCommandDecision CanStepCar(double Now, double Cooldown) const
        {
            if (HoodCounts.IsEmpty()) { return {false, TEXT("No cars in this vehicle")}; }
            if (HoodCounts.Num() <= 1) { return {false, TEXT("Only one car")}; }
            return Cooldown > 0 && Now - LastCarTime < Cooldown
                ? FDvtCommandDecision{false, TEXT("Switching\u2026")} : FDvtCommandDecision{};
        }

        FDvtCommandDecision CanStepHood(double Now, double Cooldown) const
        {
            if (HoodCount() == 0) { return {false, TEXT("No hoods")}; }
            if (HoodCount() <= 1) { return {false, TEXT("Only one hood")}; }
            return Cooldown > 0 && Now - LastHoodTime < Cooldown
                ? FDvtCommandDecision{false, TEXT("Switching\u2026")} : FDvtCommandDecision{};
        }

        bool SelectCar(int32 Index, double Now)
        {
            if (!CanSelectCar(Index)) { return false; }
            CarIndex = Index;
            ApplyRequestedHood();
            LastCarTime = Now;
            return true;
        }

        bool SelectHood(int32 Index, double Now)
        {
            if (!CanSelectHood(Index)) { return false; }
            RequestedHoodIndex = HoodIndex = Index;
            LastHoodTime = Now;
            return true;
        }

        bool StepCar(int32 Direction, double Now, double Cooldown)
        {
            if (!CanStepCar(Now, Cooldown)) { return false; }
            return SelectCar((CarIndex + Direction + HoodCounts.Num()) % HoodCounts.Num(), Now);
        }

        bool StepHood(int32 Direction, double Now, double Cooldown)
        {
            const int32 Count = HoodCount();
            if (!CanStepHood(Now, Cooldown)) { return false; }
            return SelectHood((HoodIndex + Direction + Count) % Count, Now);
        }
    };
}
