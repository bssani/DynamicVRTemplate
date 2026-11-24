#include "Grip/DvtSteeringGripComponent.h"
#include "Grip/DvtGripLogic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Features/IModularFeatures.h"
#include "GameFramework/Actor.h"
#include "HeadMountedDisplayTypes.h"
#include "IHandTracker.h"
#include "OculusXRHandComponent.h"
#if WITH_EDITOR
#include "PrimitiveSceneProxy.h"
#include "PrimitiveDrawingUtils.h"
#include "SceneManagement.h"
#include "SceneView.h"
#include "UObject/UnrealType.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogDvtGrip, Log, All);

UDvtSteeringGripComponent::UDvtSteeringGripComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bHiddenInGame = true;
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetGenerateOverlapEvents(false);
    CastShadow = false;
}

void UDvtSteeringGripComponent::BeginPlay()
{
    Super::BeginPlay();
    if (GripReleaseDistanceCm < GripEnterDistanceCm)
    {
        UE_LOG(LogDvtGrip, Warning, TEXT("Grip release %.1f cm is below enter %.1f cm; using enter as release."), GripReleaseDistanceCm, GripEnterDistanceCm);
    }
    ApplyVisuals(EControllerHand::Left, false);
    ApplyVisuals(EControllerHand::Right, false);
}

void UDvtSteeringGripComponent::SetGripHands(USkeletalMeshComponent* Left, USkeletalMeshComponent* Right)
{
    LeftGripHand = Left;
    RightGripHand = Right;
}

void UDvtSteeringGripComponent::SetTrackedHands(UOculusXRHandComponent* Left, UOculusXRHandComponent* Right)
{
    for (UOculusXRHandComponent* Old : {LeftTrackedHand.Get(), RightTrackedHand.Get()})
    {
        if (Old)
        {
            RemoveTickPrerequisiteComponent(Old);
            if (GetOwner()) { GetOwner()->RemoveTickPrerequisiteComponent(Old); }
        }
    }
    LeftTrackedHand = Left;
    RightTrackedHand = Right;
    for (UOculusXRHandComponent* Hand : {Left, Right})
    {
        if (Hand)
        {
            // Place wrists after Meta updates the bone pose (S2 D6).
            AddTickPrerequisiteComponent(Hand);
            if (GetOwner()) { GetOwner()->AddTickPrerequisiteComponent(Hand); }
        }
    }
    ApplyVisuals(EControllerHand::Left, false);
    ApplyVisuals(EControllerHand::Right, false);
}

EDvtHandGrip UDvtSteeringGripComponent::GetGrip(EControllerHand Hand) const
{
    return Hand == EControllerHand::Left ? LeftGrip : RightGrip;
}

void UDvtSteeringGripComponent::ApplyVisuals(EControllerHand Hand, bool Tracked)
{
    const bool Left = Hand == EControllerHand::Left;
    const DvtGrip::FVisibility Visible = DvtGrip::Visibility(GetGrip(Hand) == EDvtHandGrip::Gripped, Tracked);
    if (auto* Visual = Left ? LeftTrackedHand.Get() : RightTrackedHand.Get()) { Visual->SetVisibility(Visible.bTracked, true); }
    if (auto* Visual = Left ? LeftGripHand.Get() : RightGripHand.Get()) { Visual->SetVisibility(Visible.bGrip, true); }
}

void UDvtSteeringGripComponent::ApplyHandSample(EControllerHand Hand, bool TrackerValid, bool Tracked, float DistanceCm)
{
    EDvtHandGrip& Current = Hand == EControllerHand::Left ? LeftGrip : RightGrip;
    const EDvtHandGrip Next = DvtGrip::NextGrip(Current == EDvtHandGrip::Gripped, TrackerValid, Tracked,
        DistanceCm, GripEnterDistanceCm, GripReleaseDistanceCm) ? EDvtHandGrip::Gripped : EDvtHandGrip::Released;
    if (Next != Current)
    {
        Current = Next;
        const FString Detail = !TrackerValid ? TEXT("hand tracking unavailable") : !Tracked ? TEXT("hand not tracked")
            : FString::Printf(TEXT("palm %.1f cm from rim"), DistanceCm);
        UE_LOG(LogDvtGrip, Log, TEXT("%s hand %s (%s)."),
            Hand == EControllerHand::Left ? TEXT("Left") : TEXT("Right"),
            Current == EDvtHandGrip::Gripped ? TEXT("gripped") : TEXT("released"), *Detail);
        OnGripChanged.Broadcast(Hand, Current);
    }
    ApplyVisuals(Hand, Tracked);
}

void UDvtSteeringGripComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!LeftTrackedHand.IsValid() || !RightTrackedHand.IsValid())
    {
        // No pawn connection yet; keep the fixed hands visible (S2 D7).
        if (LeftGripHand) { LeftGripHand->SetVisibility(true, true); }
        if (RightGripHand) { RightGripHand->SetVisibility(true, true); }
        return;
    }
    const TArray<IHandTracker*> Trackers = IModularFeatures::Get().GetModularFeatureImplementations<IHandTracker>(IHandTracker::GetModularFeatureName());
    IHandTracker* Tracker = Trackers.IsEmpty() ? nullptr : Trackers[0];
    const bool TrackerValid = Tracker && Tracker->IsHandTrackingStateValid();
    for (EControllerHand Hand : {EControllerHand::Left, EControllerHand::Right})
    {
        TArray<FVector> Positions;
        TArray<FQuat> Rotations;
        TArray<float> Radii;
        bool Tracked = false;
        const int32 Palm = static_cast<int32>(EHandKeypoint::Palm);
        const int32 Wrist = static_cast<int32>(EHandKeypoint::Wrist);
        Tracked = TrackerValid && Tracker->GetAllKeypointStates(Hand, Positions, Rotations, Radii, Tracked)
            && Tracked && Positions.IsValidIndex(Palm) && Positions.IsValidIndex(Wrist) && Rotations.IsValidIndex(Wrist);
        const float Distance = Tracked ? DvtGrip::DistanceToRim(
            GetComponentTransform().InverseTransformPositionNoScale(Positions[Palm]), RimRadiusCm) : -1.f;
        if (Tracked)
        {
            auto* Visual = Hand == EControllerHand::Left ? LeftTrackedHand.Get() : RightTrackedHand.Get();
            // OpenXR keypoints are in world space; omit wheel scale from cm distances.
            Visual->SetWorldLocationAndRotation(Positions[Wrist], Rotations[Wrist] * TrackedHandRotationOffset.Quaternion());
        }
        ApplyHandSample(Hand, TrackerValid, Tracked, Distance);
    }
}

FBoxSphereBounds UDvtSteeringGripComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    const float Radius = RimRadiusCm + FMath::Max(GripEnterDistanceCm, GripReleaseDistanceCm);
    return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector(Radius), Radius);
}

#if WITH_EDITOR
void UDvtSteeringGripComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    UpdateBounds();
    MarkRenderStateDirty();
}

namespace
{
    class FDvtGripSceneProxy final : public FPrimitiveSceneProxy
    {
    public:
        explicit FDvtGripSceneProxy(const UDvtSteeringGripComponent* Component)
            : FPrimitiveSceneProxy(Component), Rim(Component->RimRadiusCm), Enter(Component->GripEnterDistanceCm),
              Release(FMath::Max(Component->GripEnterDistanceCm, Component->GripReleaseDistanceCm))
        {
            bWillEverBeLit = false;
        }

        virtual SIZE_T GetTypeHash() const override
        {
            static size_t Unique;
            return reinterpret_cast<SIZE_T>(&Unique);
        }

        virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
            uint32 VisibilityMap, FMeshElementCollector& Collector) const override
        {
            for (int32 Index = 0; Index < Views.Num(); ++Index)
            {
                if (!(VisibilityMap & (1u << Index))) { continue; }
                auto* PDI = Collector.GetPDI(Index);
                const FMatrix& Transform = GetLocalToWorld();
                const FVector Origin = Transform.GetOrigin();
                const FVector X = Transform.GetUnitAxis(EAxis::X);
                const FVector Y = Transform.GetUnitAxis(EAxis::Y);
                const FVector Z = Transform.GetUnitAxis(EAxis::Z);
                const auto Circle = [&](float Radius, float Axial, const FLinearColor& Color)
                {
                    DrawCircle(PDI, Origin + X * Axial, Y, Z, Color, Radius, 64, SDPG_World, 1.f);
                };
                Circle(Rim, 0, FLinearColor::White);
                const auto Envelope = [&](float Distance, const FLinearColor& Color)
                {
                    if (Rim - Distance >= 0) { Circle(Rim - Distance, 0, Color); }
                    Circle(Rim + Distance, 0, Color);
                    Circle(Rim, Distance, Color);
                    Circle(Rim, -Distance, Color);
                };
                Envelope(Enter, FLinearColor(0.1f, 1.f, 0.1f));
                Envelope(Release, FLinearColor(1.f, 0.4f, 0.f));
            }
        }

        virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
        {
            FPrimitiveViewRelevance Result;
            Result.bDrawRelevance = IsShown(View) && !View->Family->EngineShowFlags.Game;
            Result.bDynamicRelevance = true;
            Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
            return Result;
        }

        virtual uint32 GetMemoryFootprint() const override { return sizeof(*this) + GetAllocatedSize(); }
    private:
        float Rim;
        float Enter;
        float Release;
    };
}
#endif

FPrimitiveSceneProxy* UDvtSteeringGripComponent::CreateSceneProxy()
{
#if WITH_EDITOR
    if (!GetWorld() || !GetWorld()->IsGameWorld()) { return new FDvtGripSceneProxy(this); }
#endif
    return nullptr;
}
