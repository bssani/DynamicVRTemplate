#pragma once

#include "CoreMinimal.h"
#include "UI/DvtCommandDecision.h"

namespace DvtDrive
{
    inline bool HasSingleVehicle(int32 Count) { return Count == 1; }
    inline float Steering(float Value) { return FMath::Clamp(Value, -1.f, 1.f); }
    inline float Pedal(float Value) { return FMath::Clamp(Value, 0.f, 1.f); }
    inline float WheelAngle(float Value, float LockToLockDegrees) { return Value * LockToLockDegrees * 0.5f; }

    inline FTransform ResetTarget(const FVector& Location, const TArray<FTransform>& Points,
        const FTransform& Start, float LiftCm)
    {
        const FTransform* Nearest = &Start;
        double Distance = TNumericLimits<double>::Max();
        for (const FTransform& Point : Points)
        {
            const double Candidate = FVector::DistSquared(Location, Point.GetLocation());
            if (Candidate < Distance)
            {
                Distance = Candidate;
                Nearest = &Point;
            }
        }
        return FTransform(FRotator(0, Nearest->Rotator().Yaw, 0), Nearest->GetLocation() + FVector(0, 0, LiftCm));
    }

    enum class EResetPhase { Idle, FadingOut, FadingIn };

    struct FResetSequence
    {
        EResetPhase Phase = EResetPhase::Idle;
        float Remaining = 0;
        float FadeIn = 0;

        FDvtCommandDecision CanRequest() const
        {
            return Phase == EResetPhase::Idle ? FDvtCommandDecision{} : FDvtCommandDecision{false, TEXT("Reset in progress")};
        }

        bool Request(float OutSeconds, float InSeconds)
        {
            if (!CanRequest()) { return false; }
            Phase = EResetPhase::FadingOut;
            Remaining = FMath::Max(0.f, OutSeconds);
            FadeIn = FMath::Max(0.f, InSeconds);
            return true;
        }

        // Return the one teleport boundary; start fade-in after the move (S1 D6).
        bool Advance(float DeltaSeconds)
        {
            if (Phase == EResetPhase::Idle) { return false; }
            Remaining -= FMath::Max(0.f, DeltaSeconds);
            if (Remaining > 0) { return false; }
            if (Phase == EResetPhase::FadingOut)
            {
                Remaining = FadeIn;
                Phase = FadeIn > 0 ? EResetPhase::FadingIn : EResetPhase::Idle;
                return true;
            }
            Phase = EResetPhase::Idle;
            return false;
        }
    };
}
