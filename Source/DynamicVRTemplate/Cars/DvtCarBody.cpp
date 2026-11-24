#include "Cars/DvtCarBody.h"
#include "Cars/DvtHoodVariantComponent.h"
#include "Cars/DvtSelectionLogic.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"

ADvtCarBody::ADvtCarBody()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("CarOrigin"));
}

void ADvtCarBody::PrepareForSelection()
{
    bOriginalHidden = IsHidden();
    OriginalVisibility.Reset();
    TInlineComponentArray<USceneComponent*> Components(this);
    for (USceneComponent* Component : Components)
    {
        OriginalVisibility.Add({Component, Component->GetVisibleFlag()});
        // Visual bodies must never collide with their own vehicle (S3 D3).
        if (auto* Primitive = Cast<UPrimitiveComponent>(Component)) { Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision); }
    }
    GetComponents(Hoods);
    Hoods.Sort([](const UDvtHoodVariantComponent& A, const UDvtHoodVariantComponent& B)
    {
        return DvtSelection::HoodLess(A.SortOrder, A.GetName(), B.SortOrder, B.GetName());
    });
}

void ADvtCarBody::ApplyHood(int32 Index)
{
    for (const FVisibility& Saved : OriginalVisibility)
    {
        USceneComponent* Component = Saved.Component.Get();
        if (!Component) { continue; }
        bool bVisible = Saved.bVisible;
        for (int32 I = 0; I < Hoods.Num(); ++I)
        {
            if (IsValid(Hoods[I]) && I != Index && (Component == Hoods[I] || Component->IsAttachedTo(Hoods[I])))
            {
                bVisible = false;
                break;
            }
        }
        // Preserve intentionally hidden CAD parts when their variant is active (S3 D3).
        Component->SetVisibility(bVisible);
    }
}

FText ADvtCarBody::GetHoodName(int32 Index) const
{
    return Hoods.IsValidIndex(Index) && IsValid(Hoods[Index]) ? Hoods[Index]->DisplayName : FText::GetEmpty();
}

void ADvtCarBody::RestoreVisibility()
{
    SetActorHiddenInGame(bOriginalHidden);
    for (const FVisibility& Saved : OriginalVisibility)
    {
        if (Saved.Component.IsValid()) { Saved.Component->SetVisibility(Saved.bVisible); }
    }
    OriginalVisibility.Reset();
}
