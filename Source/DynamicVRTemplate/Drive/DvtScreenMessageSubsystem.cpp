#include "Drive/DvtScreenMessageSubsystem.h"
#include "CoreGlobals.h"
#include "Engine/World.h"

bool UDvtScreenMessageSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE)
        && Super::ShouldCreateSubsystem(Outer);
}

void UDvtScreenMessageSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    bPreviousMessagesEnabled = GAreScreenMessagesEnabled;
    bMessagesDisabled = true;
    GAreScreenMessagesEnabled = false;
}

void UDvtScreenMessageSubsystem::Deinitialize()
{
    // Restore only the world that changed this global (S1 D8; ReachStudy S4b).
    if (bMessagesDisabled)
    {
        GAreScreenMessagesEnabled = bPreviousMessagesEnabled;
        bMessagesDisabled = false;
    }
    Super::Deinitialize();
}
