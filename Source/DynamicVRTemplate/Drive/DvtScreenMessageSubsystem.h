#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DvtScreenMessageSubsystem.generated.h"

UCLASS()
class DYNAMICVRTEMPLATE_API UDvtScreenMessageSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;
private:
    bool bPreviousMessagesEnabled = false;
    bool bMessagesDisabled = false;
};
