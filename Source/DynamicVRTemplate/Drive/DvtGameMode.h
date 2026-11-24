#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DvtGameMode.generated.h"

UCLASS()
class DYNAMICVRTEMPLATE_API ADvtGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ADvtGameMode();
    virtual void StartPlay() override;
};
