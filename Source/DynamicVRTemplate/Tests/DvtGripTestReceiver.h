#pragma once

#include "CoreMinimal.h"
#include "Grip/DvtSteeringGripComponent.h"
#include "UObject/Object.h"
#include "DvtGripTestReceiver.generated.h"

UCLASS(Transient)
class UDvtGripTestReceiver : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION()
    void Record(EControllerHand Hand, EDvtHandGrip State)
    {
        ++Count;
        LastHand = Hand;
        LastState = State;
    }

    int32 Count = 0;
    EControllerHand LastHand = EControllerHand::Right;
    EDvtHandGrip LastState = EDvtHandGrip::Gripped;
};
