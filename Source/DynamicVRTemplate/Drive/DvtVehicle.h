#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "DvtVehicle.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class DYNAMICVRTEMPLATE_API ADvtVehicle : public AWheeledVehiclePawn
{
    GENERATED_BODY()
public:
    ADvtVehicle();
    virtual void BeginPlay() override;
    void ApplySteering(float Value);
    void TeleportForReset(const FTransform& Target);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drive")
    TObjectPtr<USceneComponent> DriverOrigin;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drive")
    TObjectPtr<USceneComponent> SteeringWheelPivot;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drive")
    TObjectPtr<UStaticMeshComponent> SteeringWheelMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drive", meta=(Units="deg", ClampMin="0"))
    float SteeringLockToLockDegrees = 900;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drive")
    TEnumAsByte<EAxis::Type> SteeringSpinAxis = EAxis::X;

    const FTransform& GetStartTransform() const { return StartTransform; }

private:
    FTransform StartTransform;
    FQuat InitialWheelRotation = FQuat::Identity;
};
