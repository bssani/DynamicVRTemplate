#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Cars/DvtSelectionLogic.h"
#include "Cars/DvtCarBody.h"
#include "Cars/DvtHoodVariantComponent.h"
#include "Drive/DvtVehicle.h"
#include "Drive/DvtVRPawn.h"
#include "Grip/DvtSteeringGripComponent.h"
#include "Tests/DvtSelectionTestReceiver.h"
#include "ChaosVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

using DvtSelection::FSelection;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtSelectionCommands, "DynamicVRTemplate.S3.E1E2E3E4.Commands", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtSelectionCommands::RunTest(const FString&)
{
    FSelection S;
    S.Initialize({3, 3, 0}, 0);
    TestFalse(TEXT("Same car does nothing"), S.SelectCar(0, 0));
    TestFalse(TEXT("High index rejected"), S.SelectCar(5, 0));
    TestFalse(TEXT("Negative index rejected"), S.SelectCar(-1, 0));
    TestEqual(TEXT("Rejected selection unchanged"), S.CarIndex, 0);
    TestTrue(TEXT("Previous wraps"), S.StepCar(-1, 0, 0.5));
    TestEqual(TEXT("Last car"), S.CarIndex, 2);
    TestTrue(TEXT("Next wraps"), S.StepCar(1, 0.5, 0.5));
    TestEqual(TEXT("First car"), S.CarIndex, 0);
    S.Initialize({2}, 0);
    TestFalse(TEXT("Single car does nothing"), S.StepCar(1, 0, 0.5));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtSelectionCooldown, "DynamicVRTemplate.S3.E5E6E7.Cooldown", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtSelectionCooldown::RunTest(const FString&)
{
    FSelection S;
    S.Initialize({3, 3, 3}, 0);
    TestTrue(TEXT("Startup has no cooldown"), S.StepCar(1, 0, 0.5));
    TestFalse(TEXT("0.3 s blocked"), S.StepCar(1, 0.3, 0.5));
    TestTrue(TEXT("Exactly 0.5 s accepted"), S.StepCar(1, 0.5, 0.5));
    TestTrue(TEXT("Independent hood cooldown"), S.StepHood(1, 0.5, 0.5));
    TestFalse(TEXT("Hood repeat blocked"), S.StepHood(1, 0.8, 0.5));
    TestTrue(TEXT("Absolute car bypasses cooldown"), S.SelectCar(0, 0.5));
    TestTrue(TEXT("Absolute hood bypasses cooldown"), S.SelectHood(2, 0.5));
    TestFalse(TEXT("Absolute change starts car cooldown"), S.StepCar(1, 0.8, 0.5));
    TestFalse(TEXT("Absolute change starts hood cooldown"), S.StepHood(1, 0.8, 0.5));
    TestTrue(TEXT("Zero disables car cooldown"), S.StepCar(1, 0.8, 0));
    TestTrue(TEXT("Zero disables hood cooldown"), S.StepHood(1, 0.8, 0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtSelectionMemory, "DynamicVRTemplate.S3.E8.HoodMemory", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtSelectionMemory::RunTest(const FString&)
{
    FSelection S;
    S.Initialize({3, 3, 2, 0}, 0);
    S.SelectHood(2, 0);
    S.SelectCar(1, 0);
    TestEqual(TEXT("Same count retains hood"), S.HoodIndex, 2);
    S.SelectCar(2, 0);
    TestEqual(TEXT("Smaller count clamps visible hood"), S.HoodIndex, 1);
    TestEqual(TEXT("Clamp keeps requested hood"), S.RequestedHoodIndex, 2);
    TestFalse(TEXT("Same visible hood is no change"), S.SelectHood(1, 0));
    TestEqual(TEXT("No-op keeps requested hood"), S.RequestedHoodIndex, 2);
    S.SelectCar(0, 0);
    TestEqual(TEXT("Returning restores requested hood"), S.HoodIndex, 2);
    S.SelectCar(3, 0);
    TestEqual(TEXT("No variants"), S.HoodIndex, -1);
    S.SelectCar(0, 0);
    TestEqual(TEXT("Return from no variants"), S.HoodIndex, 2);
    S.SelectCar(2, 0);
    S.SelectHood(0, 0);
    S.SelectHood(1, 0);
    S.SelectCar(0, 0);
    TestEqual(TEXT("Explicit changed hood remembered"), S.HoodIndex, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtHoodCommands, "DynamicVRTemplate.S3.E9.HoodCommands", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtHoodCommands::RunTest(const FString&)
{
    FSelection S;
    S.Initialize({0, 1, 3}, 0);
    TestFalse(TEXT("Empty next"), S.StepHood(1, 0, 0));
    TestFalse(TEXT("Empty direct"), S.SelectHood(0, 0));
    S.SelectCar(1, 0);
    TestFalse(TEXT("Single next"), S.StepHood(1, 0, 0));
    S.SelectCar(2, 0);
    TestFalse(TEXT("Same hood"), S.SelectHood(0, 0));
    TestFalse(TEXT("Invalid negative hood"), S.SelectHood(-1, 0));
    TestFalse(TEXT("Invalid high hood"), S.SelectHood(3, 0));
    TestTrue(TEXT("Previous hood wraps"), S.StepHood(-1, 0, 0));
    TestEqual(TEXT("Last hood"), S.HoodIndex, 2);
    TestTrue(TEXT("Next hood wraps"), S.StepHood(1, 0, 0));
    TestEqual(TEXT("First hood"), S.HoodIndex, 0);
    S.Initialize({}, 0);
    TestEqual(TEXT("Empty car index"), S.CarIndex, -1);
    TestFalse(TEXT("Empty cars"), S.StepCar(1, 0, 0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtHoodOrder, "DynamicVRTemplate.S3.E10.HoodOrder", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtHoodOrder::RunTest(const FString&)
{
    struct FKey { int32 Order; FString Name; };
    TArray<FKey> Keys = {{2, TEXT("Last")}, {0, TEXT("Zulu")}, {1, TEXT("Mid")}, {0, TEXT("Alpha")}};
    Keys.Sort([](const FKey& A, const FKey& B) { return DvtSelection::HoodLess(A.Order, A.Name, B.Order, B.Name); });
    TestEqual(TEXT("Order zero first, name breaks ties"), Keys[0].Name, FString(TEXT("Alpha")));
    TestEqual(TEXT("Other zero follows"), Keys[1].Name, FString(TEXT("Zulu")));
    TestEqual(TEXT("Order one"), Keys[2].Order, 1);
    TestEqual(TEXT("Order two"), Keys[3].Order, 2);
    return true;
}

namespace
{
    struct FCarWorld
    {
        UWorld* World;
        ADvtVehicle* Vehicle;
        ADvtCarBody* A;
        ADvtCarBody* B;
        UStaticMeshComponent* Body;
        TArray<UStaticMeshComponent*> Hoods;
        FCarWorld()
        {
            const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true)
                .ShouldSimulatePhysics(false).CreateNavigation(false).CreateAISystem(false);
            World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
            GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
            World->InitializeActorsForPlay(FURL());
            Vehicle = World->SpawnActor<ADvtVehicle>();
            A = World->SpawnActor<ADvtCarBody>();
            B = World->SpawnActor<ADvtCarBody>();
            A->AttachToActor(Vehicle, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            B->AttachToActor(Vehicle, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            A->DisplayName = FText::FromString(TEXT("Sample A"));
            B->DisplayName = FText::FromString(TEXT("Sample B"));
            Body = AddMesh(A, A->GetRootComponent());
            for (int32 Order : {2, 0, 1})
            {
                auto* Hood = NewObject<UDvtHoodVariantComponent>(A);
                A->AddInstanceComponent(Hood);
                Hood->SortOrder = Order;
                Hood->DisplayName = FText::AsNumber(Order);
                Hood->SetupAttachment(A->GetRootComponent());
                Hood->RegisterComponent();
                Hoods.Add(AddMesh(A, Hood));
            }
            Vehicle->Cars = {A, B};
        }
        UStaticMeshComponent* AddMesh(AActor* Car, USceneComponent* Parent)
        {
            auto* Mesh = NewObject<UStaticMeshComponent>(Car);
            Car->AddInstanceComponent(Mesh);
            Mesh->SetupAttachment(Parent);
            Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Mesh->RegisterComponent();
            return Mesh;
        }
        void BeginPlay()
        {
            World->BeginPlay();
            // No GameMode in this fixture to dispatch actor BeginPlay.
            World->GetWorldSettings()->NotifyBeginPlay();
        }
        ~FCarWorld()
        {
            World->DestroyWorld(false);
            GEngine->DestroyWorldContext(World);
        }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCarVisibility, "DynamicVRTemplate.S3.E11E12.VisibilityCollision", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCarVisibility::RunTest(const FString&)
{
    FCarWorld F;
    auto* BMesh = F.AddMesh(F.B, F.B->GetRootComponent());
    F.BeginPlay();
    TestFalse(TEXT("A active"), F.A->IsHidden());
    TestTrue(TEXT("B hidden"), F.B->IsHidden());
    TestTrue(TEXT("Body visible"), F.Body->IsVisible());
    TestFalse(TEXT("Order two hidden"), F.Hoods[0]->IsVisible());
    TestTrue(TEXT("Order zero visible"), F.Hoods[1]->IsVisible());
    TestFalse(TEXT("Order one hidden"), F.Hoods[2]->IsVisible());
    TestEqual(TEXT("Sorted displayed name"), F.Vehicle->GetHoodName().ToString(), FString(TEXT("0")));
    for (auto* Mesh : {F.Body, F.Hoods[0], F.Hoods[1], F.Hoods[2], BMesh})
    {
        TestTrue(TEXT("All bodies and hoods have no collision"), Mesh->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
    }
    F.Vehicle->SelectHood(2);
    TestTrue(TEXT("Selected hood visible"), F.Hoods[0]->IsVisible());
    TestFalse(TEXT("Old hood hidden"), F.Hoods[1]->IsVisible());
    TestTrue(TEXT("Chassis hidden"), F.Vehicle->GetMesh()->bHiddenInGame);
    F.Vehicle->SelectCar(1);
    TestTrue(TEXT("A hidden"), F.A->IsHidden());
    TestFalse(TEXT("B active"), F.B->IsHidden());
    TestEqual(TEXT("No hood index"), F.Vehicle->GetHoodIndex(), -1);
    TestEqual(TEXT("No hood count"), F.Vehicle->GetHoodCount(), 0);
    TestTrue(TEXT("No hood name"), F.Vehicle->GetHoodName().IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCarAuthoredVisibility, "DynamicVRTemplate.S3.E11b.AuthoredVisibility", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCarAuthoredVisibility::RunTest(const FString&)
{
    FCarWorld F;
    auto* HiddenBody = F.AddMesh(F.A, F.A->GetRootComponent());
    auto* HiddenHood = F.AddMesh(F.A, F.Hoods[1]->GetAttachParent());
    HiddenBody->SetVisibility(false);
    HiddenHood->SetVisibility(false);
    F.BeginPlay();
    TestFalse(TEXT("Authored body hidden at startup"), HiddenBody->GetVisibleFlag());
    TestFalse(TEXT("Authored hood child hidden at startup"), HiddenHood->GetVisibleFlag());
    TestTrue(TEXT("Other active hood child visible"), F.Hoods[1]->GetVisibleFlag());
    F.Vehicle->SelectHood(1);
    TestFalse(TEXT("Inactive hood child hidden"), F.Hoods[1]->GetVisibleFlag());
    TestFalse(TEXT("Authored body stays hidden on switch"), HiddenBody->GetVisibleFlag());
    F.Vehicle->SelectHood(0);
    TestFalse(TEXT("Authored hood child stays hidden on return"), HiddenHood->GetVisibleFlag());
    TestFalse(TEXT("Authored body stays hidden on return"), HiddenBody->GetVisibleFlag());
    TestTrue(TEXT("Other active hood child restored"), F.Hoods[1]->GetVisibleFlag());
    F.Vehicle->SelectCar(1);
    F.Vehicle->SelectCar(0);
    TestFalse(TEXT("Authored body stays hidden on car return"), HiddenBody->GetVisibleFlag());
    TestFalse(TEXT("Authored hood child stays hidden on car return"), HiddenHood->GetVisibleFlag());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCarWheel, "DynamicVRTemplate.S3.E13.WheelAndHands", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCarWheel::RunTest(const FString&)
{
    FCarWorld F;
    F.B->SteeringWheelMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    F.B->SteeringWheelMeshTransform = FTransform(FRotator(12, 23, 34), FVector(3, 4, 5), FVector(0.2, 0.3, 0.4));
    F.B->RimRadiusCm = 23;
    F.B->LeftGripHandTransform = FTransform(FVector(0, -23, 0));
    F.B->RightGripHandTransform = FTransform(FVector(0, 23, 0));
    F.BeginPlay();
    F.Vehicle->ApplySteering(0.4f);
    const FTransform Pivot = F.Vehicle->SteeringWheelPivot->GetRelativeTransform();
    const float Enter = F.Vehicle->SteeringGrip->GripEnterDistanceCm;
    const float Release = F.Vehicle->SteeringGrip->GripReleaseDistanceCm;
    F.Vehicle->SelectCar(1);
    TestTrue(TEXT("Wheel mesh"), F.Vehicle->SteeringWheelMesh->GetStaticMesh() == F.B->SteeringWheelMesh);
    TestTrue(TEXT("Wheel transform"), F.Vehicle->SteeringWheelMesh->GetRelativeTransform().Equals(F.B->SteeringWheelMeshTransform));
    TestEqual(TEXT("Radius cm"), F.Vehicle->SteeringGrip->RimRadiusCm, 23.f);
    TestTrue(TEXT("Left hand transform"), F.Vehicle->LeftGripHand->GetRelativeTransform().Equals(F.B->LeftGripHandTransform));
    TestTrue(TEXT("Right hand transform"), F.Vehicle->RightGripHand->GetRelativeTransform().Equals(F.B->RightGripHandTransform));
    TestTrue(TEXT("Pivot preserved while steering"), F.Vehicle->SteeringWheelPivot->GetRelativeTransform().Equals(Pivot));
    TestEqual(TEXT("Enter unchanged"), F.Vehicle->SteeringGrip->GripEnterDistanceCm, Enter);
    TestEqual(TEXT("Release unchanged"), F.Vehicle->SteeringGrip->GripReleaseDistanceCm, Release);
    F.Vehicle->SelectCar(0);
    TestFalse(TEXT("Missing wheel hidden"), F.Vehicle->SteeringWheelMesh->IsVisible());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCarSanitize, "DynamicVRTemplate.S3.E14E15.EmptyAndSanitize", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCarSanitize::RunTest(const FString&)
{
    FCarWorld F;
    F.Vehicle->Cars = {nullptr, F.A, F.A, F.B};
    AddExpectedError(TEXT("null car"), EAutomationExpectedErrorFlags::Contains, 1);
    AddExpectedError(TEXT("duplicate car"), EAutomationExpectedErrorFlags::Contains, 1);
    F.BeginPlay();
    TestEqual(TEXT("Filtered count"), F.Vehicle->GetCarCount(), 2);
    TestEqual(TEXT("Filtered order"), F.Vehicle->Cars[0].Get(), F.A);
    TestEqual(TEXT("Car name"), F.Vehicle->GetCarName().ToString(), FString(TEXT("Sample A")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCarEmpty, "DynamicVRTemplate.S3.E14.Empty", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCarEmpty::RunTest(const FString&)
{
    FCarWorld F;
    F.Vehicle->Cars.Reset();
    auto* ExtraBody = F.AddMesh(F.Vehicle, F.Vehicle->GetMesh());
    F.BeginPlay();
    F.Vehicle->NextCar(); F.Vehicle->PreviousCar(); F.Vehicle->SelectCar(0);
    F.Vehicle->NextHood(); F.Vehicle->PreviousHood(); F.Vehicle->SelectHood(0);
    TestEqual(TEXT("No car selected"), F.Vehicle->GetCarIndex(), -1);
    TestEqual(TEXT("No cars"), F.Vehicle->GetCarCount(), 0);
    TestTrue(TEXT("Chassis visible without cars"), !F.Vehicle->GetMesh()->bHiddenInGame);
    TestFalse(TEXT("Additional body not hidden without cars"), ExtraBody->bHiddenInGame);
    TestTrue(TEXT("Additional body visible without cars"), ExtraBody->GetVisibleFlag());
    TestTrue(TEXT("Empty name"), F.Vehicle->GetCarName().IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtVehicleBodyVisibility, "DynamicVRTemplate.S3.E14b.VehicleBodyVisibility", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtVehicleBodyVisibility::RunTest(const FString&)
{
    for (bool HideBody : {true, false})
    {
        FCarWorld F;
        F.Vehicle->bHideChassisWhenCarsExist = HideBody;
        auto* VisibleBody = F.AddMesh(F.Vehicle, F.Vehicle->GetMesh());
        auto* HiddenBody = F.AddMesh(F.Vehicle, F.Vehicle->GetMesh());
        HiddenBody->SetHiddenInGame(true);
        const auto ChassisCollision = F.Vehicle->GetMesh()->GetCollisionEnabled();
        const auto BodyCollision = VisibleBody->GetCollisionEnabled();
        F.BeginPlay();
        TestEqual(TEXT("Chassis hidden only when enabled"), bool(F.Vehicle->GetMesh()->bHiddenInGame), HideBody);
        TestEqual(TEXT("Additional body hidden only when enabled"), bool(VisibleBody->bHiddenInGame), HideBody);
        TestTrue(TEXT("Authored hidden body stays hidden"), HiddenBody->bHiddenInGame);
        TestFalse(TEXT("Wheel not hidden"), F.Vehicle->SteeringWheelMesh->bHiddenInGame);
        TestFalse(TEXT("Left hand not hidden"), F.Vehicle->LeftGripHand->bHiddenInGame);
        TestFalse(TEXT("Right hand not hidden"), F.Vehicle->RightGripHand->bHiddenInGame);
        TestTrue(TEXT("Grip keeps its own game hidden flag"), F.Vehicle->SteeringGrip->bHiddenInGame);
        TestFalse(TEXT("Other actor primitive not hidden"), F.Body->bHiddenInGame);
        TestTrue(TEXT("Chassis collision unchanged"), F.Vehicle->GetMesh()->GetCollisionEnabled() == ChassisCollision);
        TestTrue(TEXT("Body collision unchanged"), VisibleBody->GetCollisionEnabled() == BodyCollision);
        F.Vehicle->RouteEndPlay(EEndPlayReason::EndPlayInEditor);
        TestFalse(TEXT("Chassis original hidden flag restored"), F.Vehicle->GetMesh()->bHiddenInGame);
        TestFalse(TEXT("Visible body original hidden flag restored"), VisibleBody->bHiddenInGame);
        TestTrue(TEXT("Hidden body original hidden flag restored"), HiddenBody->bHiddenInGame);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCarUnchangedPhysics, "DynamicVRTemplate.S3.E16.PhysicsAndEvents", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCarUnchangedPhysics::RunTest(const FString&)
{
    FCarWorld F;
    auto* Receiver = NewObject<UDvtSelectionTestReceiver>();
    F.Vehicle->OnSelectionChanged.AddDynamic(Receiver, &UDvtSelectionTestReceiver::Record);
    F.BeginPlay();
    TestEqual(TEXT("No startup event"), Receiver->Count, 0);
    F.Vehicle->SelectCar(0);
    TestEqual(TEXT("No same-car event"), Receiver->Count, 0);
    AddExpectedError(TEXT("Invalid car index"), EAutomationExpectedErrorFlags::Contains, 2);
    F.Vehicle->SelectCar(-1); F.Vehicle->SelectCar(5);
    TestEqual(TEXT("No invalid-car event"), Receiver->Count, 0);
    auto* Pawn = F.World->SpawnActor<ADvtVRPawn>();
    Pawn->AttachToComponent(F.Vehicle->DriverOrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    const FTransform Before = F.Vehicle->GetActorTransform();
    const FTransform PawnBefore = Pawn->GetActorTransform();
    const auto Collision = F.Vehicle->GetMesh()->GetCollisionEnabled();
    const bool Simulates = F.Vehicle->GetMesh()->IsSimulatingPhysics();
    auto* Movement = F.Vehicle->GetVehicleMovementComponent();
    Movement->SetThrottleInput(0.7f); Movement->SetBrakeInput(0.2f);
    F.Vehicle->ApplySteering(0.4f);
    F.Vehicle->SelectHood(2);
    TestEqual(TEXT("One hood event"), Receiver->Count, 1);
    F.Vehicle->SelectHood(2);
    TestEqual(TEXT("No same-hood event"), Receiver->Count, 1);
    F.Vehicle->SelectCar(1);
    TestEqual(TEXT("Car plus hood emits once"), Receiver->Count, 2);
    TestEqual(TEXT("Event car"), Receiver->LastCar, 1);
    TestEqual(TEXT("Event hood"), Receiver->LastHood, -1);
    TestTrue(TEXT("Vehicle transform preserved"), F.Vehicle->GetActorTransform().Equals(Before));
    TestTrue(TEXT("Pawn world transform preserved"), Pawn->GetActorTransform().Equals(PawnBefore));
    TestEqual(TEXT("Pawn parent preserved"), Pawn->GetRootComponent()->GetAttachParent(), F.Vehicle->DriverOrigin.Get());
    TestTrue(TEXT("Physics collision preserved"), Collision == F.Vehicle->GetMesh()->GetCollisionEnabled());
    TestEqual(TEXT("Simulation preserved"), F.Vehicle->GetMesh()->IsSimulatingPhysics(), Simulates);
    TestEqual(TEXT("Throttle preserved"), Movement->GetThrottleInput(), 0.7f);
    TestEqual(TEXT("Brake preserved"), Movement->GetBrakeInput(), 0.2f);
    TestEqual(TEXT("Steering preserved"), Movement->GetSteeringInput(), 0.4f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCarRestore, "DynamicVRTemplate.S3.E17.RestoreVisibility", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCarRestore::RunTest(const FString&)
{
    FCarWorld F;
    F.A->SetActorHiddenInGame(true);
    F.Hoods[0]->SetVisibility(false);
    F.Hoods[1]->SetVisibility(true);
    F.Hoods[2]->SetVisibility(false);
    F.Body->SetVisibility(false);
    F.BeginPlay();
    F.Vehicle->SelectHood(2);
    F.Vehicle->SelectCar(1);
    F.Vehicle->RouteEndPlay(EEndPlayReason::EndPlayInEditor);
    TestTrue(TEXT("A hidden restored"), F.A->IsHidden());
    TestFalse(TEXT("B visible restored"), F.B->IsHidden());
    TestFalse(TEXT("First hood hidden restored"), F.Hoods[0]->IsVisible());
    TestTrue(TEXT("Second hood visible restored"), F.Hoods[1]->IsVisible());
    TestFalse(TEXT("Third hood hidden restored"), F.Hoods[2]->IsVisible());
    TestFalse(TEXT("Body original visibility"), F.Body->IsVisible());
    TestFalse(TEXT("Chassis restored"), F.Vehicle->GetMesh()->bHiddenInGame);
    TestTrue(TEXT("Collision stays off"), F.Body->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
    return true;
}
#endif
