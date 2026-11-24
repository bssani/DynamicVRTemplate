#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DvtSelectionTestReceiver.generated.h"

UCLASS(Transient)
class UDvtSelectionTestReceiver : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION()
    void Record(int32 Car, int32 Hood) { ++Count; LastCar = Car; LastHood = Hood; }
    int32 Count = 0;
    int32 LastCar = -1;
    int32 LastHood = -1;
};
