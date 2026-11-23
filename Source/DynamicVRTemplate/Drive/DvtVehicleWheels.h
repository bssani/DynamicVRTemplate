#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "DvtVehicleWheels.generated.h"

UCLASS()
class DYNAMICVRTEMPLATE_API UDvtWheelFront : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    UDvtWheelFront();
};

UCLASS()
class DYNAMICVRTEMPLATE_API UDvtWheelRear : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    UDvtWheelRear();
};
