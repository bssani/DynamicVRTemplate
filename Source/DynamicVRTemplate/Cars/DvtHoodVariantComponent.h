#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DvtHoodVariantComponent.generated.h"

UCLASS(ClassGroup=(Cars), meta=(BlueprintSpawnableComponent))
class DYNAMICVRTEMPLATE_API UDvtHoodVariantComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars")
    FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars")
    int32 SortOrder = 0;
};
