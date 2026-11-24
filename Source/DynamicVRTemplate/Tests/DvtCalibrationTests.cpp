#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Calibration/DvtCalibrationLogic.h"
#include "Calibration/DvtCalibrationGameMode.h"
#include "Drive/DvtGameMode.h"
#include "Drive/DvtPlayerController.h"
#include "Drive/DvtVehicle.h"
#include "Drive/DvtVRPawn.h"
#include "VRCalAlignmentComponent.h"
#include "VRCalAlignPointComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/WorldSettings.h"

struct FDvtCalibrationTestAccess
{
    static void Attach(ADvtPlayerController* Controller, ADvtVehicle* Vehicle)
    {
        Controller->Vehicle = Vehicle;
        Controller->AttachVRPawn();
    }
    static void InterceptReapply(ADvtPlayerController* Controller, TFunction<bool(UVRCalAlignmentComponent*)> Callback)
    {
        Controller->ReapplyForTest = MoveTemp(Callback);
    }
};

namespace
{
    struct FDvtCalibrationWorld
    {
        UWorld* World;
        explicit FDvtCalibrationWorld(bool bWithGameMode = false)
        {
            const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false)
                .CreatePhysicsScene(true).ShouldSimulatePhysics(false).EnableTraceCollision(true)
                .CreateNavigation(false).CreateAISystem(false);
            World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
            GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
            FURL URL;
            if (bWithGameMode)
            {
                World->SetGameInstance(NewObject<UGameInstance>(GEngine));
                URL.AddOption(TEXT("game=/Script/DynamicVRTemplate.DvtGameMode"));
                World->SetGameMode(URL);
            }
            World->InitializeActorsForPlay(URL);
        }
        ~FDvtCalibrationWorld()
        {
            World->DestroyWorld(false);
            GEngine->DestroyWorldContext(World);
        }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCalibrationDefaults, "DynamicVRTemplate.S4.E1.Components", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCalibrationDefaults::RunTest(const FString&)
{
    const auto* Vehicle = GetDefault<ADvtVehicle>();
    TArray<UVRCalAlignmentComponent*> Alignments;
    TArray<UVRCalAlignPointComponent*> Points;
    Vehicle->GetComponents(Alignments);
    Vehicle->GetComponents(Points);
    TestEqual(TEXT("One alignment"), Alignments.Num(), 1);
    TestEqual(TEXT("Four points"), Points.Num(), 4);
    TSet<int32> Orders;
    for (const auto* Point : Points)
    {
        TestTrue(TEXT("Order in 1-4"), Point->Order >= 1 && Point->Order <= 4);
        TestTrue(TEXT("Point under vehicle root"), Point->GetAttachParent() == Vehicle->GetRootComponent());
        Orders.Add(Point->Order);
    }
    TestEqual(TEXT("Unique orders"), Orders.Num(), 4);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCalibrationAttachDecision, "DynamicVRTemplate.S4.E2E3E4.AttachDecision", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCalibrationAttachDecision::RunTest(const FString&)
{
    FDvtCalibrationWorld F;
    auto* Vehicle = F.World->SpawnActor<ADvtVehicle>();
    auto* Pawn = F.World->SpawnActor<ADvtVRPawn>();
    auto* Other = F.World->SpawnActor<ADvtVRPawn>();
    TestTrue(TEXT("No parent uses fallback"), DvtCalibration::ShouldAttachToDriver(Pawn->GetAttachParentActor(), Vehicle));
    Pawn->AttachToActor(Vehicle, FAttachmentTransformRules::KeepWorldTransform);
    TestFalse(TEXT("Root attachment preserved"), DvtCalibration::ShouldAttachToDriver(Pawn->GetAttachParentActor(), Vehicle));
    Pawn->AttachToComponent(Vehicle->DriverOrigin, FAttachmentTransformRules::KeepWorldTransform);
    TestFalse(TEXT("Driver attachment preserved"), DvtCalibration::ShouldAttachToDriver(Pawn->GetAttachParentActor(), Vehicle));
    Pawn->AttachToActor(Other, FAttachmentTransformRules::KeepWorldTransform);
    TestTrue(TEXT("Other parent uses fallback"), DvtCalibration::ShouldAttachToDriver(Pawn->GetAttachParentActor(), Vehicle));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCalibrationAttachOrder, "DynamicVRTemplate.S4.E5E6.AttachOrder", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCalibrationAttachOrder::RunTest(const FString&)
{
    FDvtCalibrationWorld F;
    auto* Vehicle = F.World->SpawnActor<ADvtVehicle>();
    auto* Pawn = F.World->SpawnActor<ADvtVRPawn>();
    auto* Controller = F.World->SpawnActor<ADvtPlayerController>();
    Controller->Possess(Pawn);
    Vehicle->DriverOrigin->SetRelativeLocation(FVector(10, 20, 30));
    const FTransform T(FRotator(0, 17, 0), FVector(31, -24, 9));
    Pawn->AttachToActor(Vehicle, FAttachmentTransformRules::KeepWorldTransform);
    Pawn->SetActorRelativeTransform(T);
    FDvtCalibrationTestAccess::Attach(Controller, Vehicle);
    TestTrue(TEXT("Kit first keeps root parent"), Pawn->GetRootComponent()->GetAttachParent() == Vehicle->GetRootComponent());
    TestTrue(TEXT("Kit first keeps transform"), Pawn->GetRootComponent()->GetRelativeTransform().Equals(T));
    Pawn->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    FDvtCalibrationTestAccess::Attach(Controller, Vehicle);
    TestTrue(TEXT("Controller first uses driver origin"), Pawn->GetRootComponent()->GetAttachParent() == Vehicle->DriverOrigin);
    TestTrue(TEXT("Fallback snaps to driver"), Pawn->GetRootComponent()->GetRelativeTransform().Equals(FTransform::Identity));
    Pawn->AttachToActor(Vehicle, FAttachmentTransformRules::KeepWorldTransform);
    Pawn->SetActorRelativeTransform(T);
    FDvtCalibrationTestAccess::Attach(Controller, Vehicle);
    TestTrue(TEXT("Later calibration keeps root parent"), Pawn->GetRootComponent()->GetAttachParent() == Vehicle->GetRootComponent());
    TestTrue(TEXT("Later calibration keeps transform"), Pawn->GetRootComponent()->GetRelativeTransform().Equals(T));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCalibrationInput, "DynamicVRTemplate.S4.E7.ReapplyInput", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCalibrationInput::RunTest(const FString&)
{
    FDvtCalibrationWorld F;
    auto* Vehicle = F.World->SpawnActor<ADvtVehicle>();
    auto* Controller = F.World->SpawnActor<ADvtPlayerController>();
    FDvtCalibrationTestAccess::Attach(Controller, Vehicle);
    Controller->ReapplyAlignmentAction = NewObject<UInputAction>(Controller);
    auto* Inputs = NewObject<UEnhancedInputComponent>(Controller);
    Controller->InputComponent = Inputs;
    int32 Calls = 0;
    FDvtCalibrationTestAccess::InterceptReapply(Controller, [&](UVRCalAlignmentComponent* Alignment)
    {
        TestTrue(TEXT("Vehicle alignment requested"), Alignment == Vehicle->Alignment);
        ++Calls;
        return true;
    });
    Controller->SetupInputComponent();
    int32 Bindings = 0;
    for (const auto& Binding : Inputs->GetActionEventBindings())
    {
        if (Binding->GetAction() == Controller->ReapplyAlignmentAction)
        {
            TestTrue(TEXT("Started event"), Binding->GetTriggerEvent() == ETriggerEvent::Started);
            Binding->Execute(FInputActionInstance(Controller->ReapplyAlignmentAction));
            ++Bindings;
        }
    }
    TestEqual(TEXT("One binding"), Bindings, 1);
    TestEqual(TEXT("One request"), Calls, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCalibrationMode, "DynamicVRTemplate.S4.E8.CalibrationMode", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCalibrationMode::RunTest(const FString&)
{
    const auto* Mode = GetDefault<ADvtCalibrationGameMode>();
    TestTrue(TEXT("Engine controller without driving"), Mode->PlayerControllerClass == APlayerController::StaticClass());
    TestTrue(TEXT("VR pawn"), Mode->DefaultPawnClass == ADvtVRPawn::StaticClass());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtCalibrationSetup, "DynamicVRTemplate.S4.E10E11.Setup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtCalibrationSetup::RunTest(const FString&)
{
    const auto* Pawn = GetDefault<ADvtVRPawn>();
    TestNotNull(TEXT("Pawn root"), Pawn->GetRootComponent());
    TestFalse(TEXT("No controller yaw"), Pawn->bUseControllerRotationYaw);
    const UClass* Sample = LoadClass<ADvtVehicle>(nullptr, TEXT("/Game/Samples/Blueprints/BP_SampleVehicle.BP_SampleVehicle_C"));
    if (!TestNotNull(TEXT("Sample class"), Sample)) { return false; }
    const auto* Root = Sample->GetDefaultObject<ADvtVehicle>()->GetRootComponent();
    if (!TestNotNull(TEXT("Sample root"), Root)) { return false; }
    TestTrue(TEXT("Sample unit root scale"), Root->GetRelativeScale3D().Equals(FVector::OneVector));
    FDvtCalibrationWorld F(true);
    F.World->GetWorldSettings()->WorldToMeters = 73;
    auto* Mode = Cast<ADvtGameMode>(F.World->GetAuthGameMode());
    if (!TestNotNull(TEXT("Drive game mode"), Mode)) { return false; }
    Mode->StartPlay();
    TestEqual(TEXT("Game mode sets cm scale"), F.World->GetWorldSettings()->WorldToMeters, 100.f);
    return true;
}
#endif
