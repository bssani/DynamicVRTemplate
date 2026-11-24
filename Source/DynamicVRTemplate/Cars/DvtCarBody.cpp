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
    TInlineComponentArray<USceneComponent*> Components(this);
    for (USceneComponent* Component : Components) { Component->SetVisibility(true); }
    for (int32 I = 0; I < Hoods.Num(); ++I)
    {
        if (IsValid(Hoods[I])) { Hoods[I]->SetVisibility(I == Index, true); }
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
