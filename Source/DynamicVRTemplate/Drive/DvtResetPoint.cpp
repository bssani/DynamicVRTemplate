#include "Drive/DvtResetPoint.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

ADvtResetPoint::ADvtResetPoint()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
#if WITH_EDITORONLY_DATA
    UArrowComponent* Arrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Direction"));
    if (Arrow)
    {
        Arrow->SetupAttachment(RootComponent);
        Arrow->SetHiddenInGame(true);
        Arrow->ArrowSize = 3;
    }
#endif
}
