#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "InputCoreTypes.h"
#include "DvtSteeringGripComponent.generated.h"

class UOculusXRHandComponent;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EDvtHandGrip : uint8
{
    Gripped,
    Released
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDvtGripChanged, EControllerHand, Hand, EDvtHandGrip, State);

UCLASS(ClassGroup=(VR), meta=(BlueprintSpawnableComponent))
class DYNAMICVRTEMPLATE_API UDvtSteeringGripComponent : public UPrimitiveComponent
{
    GENERATED_BODY()
public:
    UDvtSteeringGripComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR|Grip", meta=(Units="cm", ClampMin="0", ClampMax="50"))
    float RimRadiusCm = 18;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR|Grip", meta=(Units="cm", ClampMin="0", ClampMax="50"))
    float GripEnterDistanceCm = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR|Grip", meta=(Units="cm", ClampMin="0", ClampMax="50"))
    float GripReleaseDistanceCm = 18;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VR|Grip", meta=(Units="deg"))
    FRotator TrackedHandRotationOffset = FRotator::ZeroRotator;
    UPROPERTY(BlueprintAssignable, Category="VR|Grip")
    FDvtGripChanged OnGripChanged;

    UFUNCTION(BlueprintPure, Category="VR|Grip")
    EDvtHandGrip GetGrip(EControllerHand Hand) const;

    void SetGripHands(USkeletalMeshComponent* Left, USkeletalMeshComponent* Right);
    void SetTrackedHands(UOculusXRHandComponent* Left, UOculusXRHandComponent* Right);

private:
    friend class FDvtGripIndependentEvent;
    void ApplyHandSample(EControllerHand Hand, bool TrackerValid, bool Tracked, float DistanceCm);
    void ApplyVisuals(EControllerHand Hand, bool Tracked);

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> LeftGripHand;
    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> RightGripHand;
    UPROPERTY(Transient)
    TWeakObjectPtr<UOculusXRHandComponent> LeftTrackedHand;
    UPROPERTY(Transient)
    TWeakObjectPtr<UOculusXRHandComponent> RightTrackedHand;
    EDvtHandGrip LeftGrip = EDvtHandGrip::Gripped;
    EDvtHandGrip RightGrip = EDvtHandGrip::Gripped;
};
