#include "Drive/DvtVehicleWheels.h"

UDvtWheelFront::UDvtWheelFront()
{
    // Epic Vehicle template front wheel (S1 D1).
    AxleType = EAxleType::Front;
    bAffectedBySteering = true;
    MaxSteerAngle = 40;
    WheelRadius = 39;
    WheelWidth = 35;
    FrictionForceMultiplier = 3;
    MaxBrakeTorque = 4500;
    MaxHandBrakeTorque = 6000;
}

UDvtWheelRear::UDvtWheelRear()
{
    // Epic Vehicle template rear wheel (S1 D1).
    AxleType = EAxleType::Rear;
    bAffectedByEngine = true;
    bAffectedByHandbrake = true;
    WheelRadius = 40;
    WheelWidth = 40;
    FrictionForceMultiplier = 4;
    SlipThreshold = 100;
    SkidThreshold = 100;
    MaxSteerAngle = 0;
    MaxHandBrakeTorque = 6000;
}
