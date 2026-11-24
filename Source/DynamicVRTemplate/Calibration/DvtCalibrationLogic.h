#pragma once

class AActor;

namespace DvtCalibration
{
    inline bool ShouldAttachToDriver(const AActor* Parent, const AActor* Vehicle)
    {
        return Parent != Vehicle;
    }
}
