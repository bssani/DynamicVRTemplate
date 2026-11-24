#pragma once

#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"

namespace DvtGrip
{
    inline float DistanceToRim(const FVector& LocalPoint, float RadiusCm)
    {
        const double Radial = FMath::Sqrt(LocalPoint.Y * LocalPoint.Y + LocalPoint.Z * LocalPoint.Z) - RadiusCm;
        return static_cast<float>(FMath::Sqrt(LocalPoint.X * LocalPoint.X + Radial * Radial));
    }

    inline bool NextGrip(bool Gripped, bool TrackerValid, bool Tracked, float DistanceCm, float EnterCm, float ReleaseCm)
    {
        if (!TrackerValid) { return true; }
        // Hold grip while tracking is lost (fist hides the hand) (D-05).
        if (!Tracked) { return Gripped; }
        return Gripped ? DistanceCm <= FMath::Max(EnterCm, ReleaseCm) : DistanceCm < EnterCm;
    }

    struct FVisibility
    {
        bool bTracked;
        bool bGrip;
    };

    inline FVisibility Visibility(bool Gripped, bool Tracked)
    {
        return {!Gripped && Tracked, Gripped};
    }
}
