#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DvtCarBody.generated.h"

class UStaticMesh;
class UDvtHoodVariantComponent;
class USceneComponent;

UCLASS()
class DYNAMICVRTEMPLATE_API ADvtCarBody : public AActor
{
    GENERATED_BODY()
public:
    ADvtCarBody();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars")
    FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars|Wheel")
    TObjectPtr<UStaticMesh> SteeringWheelMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars|Wheel")
    FTransform SteeringWheelMeshTransform;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars|Wheel", meta=(Units="cm", ClampMin="0"))
    float RimRadiusCm = 18;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars|Wheel")
    FTransform LeftGripHandTransform;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars|Wheel")
    FTransform RightGripHandTransform;

    void PrepareForSelection();
    void ApplyHood(int32 Index);
    void RestoreVisibility();
    int32 GetHoodCount() const { return Hoods.Num(); }
    FText GetHoodName(int32 Index) const;

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<UDvtHoodVariantComponent>> Hoods;
    struct FVisibility
    {
        TWeakObjectPtr<USceneComponent> Component;
        bool bVisible;
    };
    TArray<FVisibility> OriginalVisibility;
    bool bOriginalHidden = false;
};
