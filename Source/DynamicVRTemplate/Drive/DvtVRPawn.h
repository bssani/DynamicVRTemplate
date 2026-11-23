#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DvtVRPawn.generated.h"

class UCameraComponent;

UCLASS()
class DYNAMICVRTEMPLATE_API ADvtVRPawn : public APawn
{
    GENERATED_BODY()
public:
    ADvtVRPawn();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VR")
    TObjectPtr<UCameraComponent> Camera;
};
