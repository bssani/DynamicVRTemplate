#pragma once

#include "CoreMinimal.h"

struct FDvtCommandDecision
{
    bool bAccepted = true;
    FString Reason;
    operator bool() const { return bAccepted; }
};
