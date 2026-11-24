#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DvtVRPawn.generated.h"

class UCameraComponent;
class UOculusXRHandComponent;

UCLASS()
class DYNAMICVRTEMPLATE_API ADvtVRPawn : public APawn
{
    GENERATED_BODY()
public:
    ADvtVRPawn();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VR")
    TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VR")
    TObjectPtr<UOculusXRHandComponent> LeftTrackedHand;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VR")
    TObjectPtr<UOculusXRHandComponent> RightTrackedHand;
};
