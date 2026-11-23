#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Drive/DvtDriveLogic.h"
#include "Drive/DvtVehicle.h"
#include "Drive/DvtPlayerController.h"
#include "Drive/DvtScreenMessageSubsystem.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "CoreGlobals.h"
#include "InputActionValue.h"
#include "UObject/UnrealType.h"

namespace
{
    struct FDvtTestWorld
    {
        UWorld* World;
        explicit FDvtTestWorld(EWorldType::Type Type = EWorldType::Game)
        {
            const UWorld::InitializationValues Values = UWorld::InitializationValues()
                .AllowAudioPlayback(false).CreatePhysicsScene(true).ShouldSimulatePhysics(false)
                .EnableTraceCollision(true).CreateNavigation(false).CreateAISystem(false);
            World = UWorld::CreateWorld(Type, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
            GEngine->CreateNewWorldContext(Type).SetCurrentWorld(World);
            if (Type != EWorldType::Editor) { World->InitializeActorsForPlay(FURL()); }
        }
        ~FDvtTestWorld()
        {
            World->DestroyWorld(false);
            GEngine->DestroyWorldContext(World);
        }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtDefaults, "DynamicVRTemplate.S1.E1.VehicleDefaults", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtDefaults::RunTest(const FString&)
{
    const ADvtVehicle* Vehicle = GetDefault<ADvtVehicle>();
    const FBoolProperty* RequiresController = FindFProperty<FBoolProperty>(UChaosVehicleMovementComponent::StaticClass(), TEXT("bRequiresControllerForInputs"));
    if (!TestNotNull(TEXT("Controller requirement property"), RequiresController)) { return false; }
    TestFalse(TEXT("No controller required"), RequiresController->GetPropertyValue_InContainer(Vehicle->GetVehicleMovementComponent()));
    TestTrue(TEXT("No player possession"), Vehicle->AutoPossessPlayer == EAutoReceiveInput::Disabled);
    TestTrue(TEXT("No AI possession"), Vehicle->AutoPossessAI == EAutoPossessAI::Disabled);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtInputs, "DynamicVRTemplate.S1.E2E3.InputForwarding", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtInputs::RunTest(const FString&)
{
    FDvtTestWorld Fixture;
    UWorld* World = Fixture.World;
    ADvtVehicle* Vehicle = World->SpawnActor<ADvtVehicle>();
    ADvtPlayerController* Controller = World->SpawnActor<ADvtPlayerController>();
    Controller->SetDriveVehicleForTest(Vehicle);
    auto* Movement = Vehicle->GetVehicleMovementComponent();
    Controller->Steer(FInputActionValue(1.7f));
    TestEqual(TEXT("Steer upper"), Movement->GetSteeringInput(), 1.f);
    Controller->Steer(FInputActionValue(-2.f));
    TestEqual(TEXT("Steer lower"), Movement->GetSteeringInput(), -1.f);
    Controller->Throttle(FInputActionValue(-0.3f));
    TestEqual(TEXT("Throttle lower"), Movement->GetThrottleInput(), 0.f);
    Controller->Throttle(FInputActionValue(1.2f));
    TestEqual(TEXT("Throttle upper"), Movement->GetThrottleInput(), 1.f);
    Controller->Brake(FInputActionValue(-0.3f));
    TestEqual(TEXT("Brake lower"), Movement->GetBrakeInput(), 0.f);
    Controller->Brake(FInputActionValue(1.2f));
    TestEqual(TEXT("Brake upper"), Movement->GetBrakeInput(), 1.f);
    Controller->SteerCompleted(FInputActionValue());
    Controller->ThrottleCompleted(FInputActionValue());
    Controller->BrakeCompleted(FInputActionValue());
    TestEqual(TEXT("Steer released"), Movement->GetSteeringInput(), 0.f);
    TestEqual(TEXT("Throttle released"), Movement->GetThrottleInput(), 0.f);
    TestEqual(TEXT("Brake released"), Movement->GetBrakeInput(), 0.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtWheelAngle, "DynamicVRTemplate.S1.E4.WheelAngle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtWheelAngle::RunTest(const FString&)
{
    TestEqual(TEXT("Center"), DvtDrive::WheelAngle(0, 900), 0.f);
    TestEqual(TEXT("Right"), DvtDrive::WheelAngle(1, 900), 450.f);
    TestEqual(TEXT("Left"), DvtDrive::WheelAngle(-1, 900), -450.f);
    TestEqual(TEXT("Half"), DvtDrive::WheelAngle(0.5f, 900), 225.f);
    TestEqual(TEXT("540 deg"), DvtDrive::WheelAngle(1, 540), 270.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtResetTarget, "DynamicVRTemplate.S1.E5E6E7.ResetTarget", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtResetTarget::RunTest(const FString&)
{
    const FTransform Start(FRotator(20, 45, 10), FVector(10, 20, 30));
    FTransform Target = DvtDrive::ResetTarget(FVector::ZeroVector, {}, Start, 20);
    TestTrue(TEXT("Start plus lift"), Target.GetLocation().Equals(FVector(10, 20, 50)));
    TestTrue(TEXT("Fallback yaw only"), Target.Rotator().Equals(FRotator(0, 45, 0)));
    const TArray<FTransform> Points = {
        FTransform(FVector(1000, 0, 0)), FTransform(FRotator(30, 90, 10), FVector(-300, 0, 0)), FTransform(FVector(0, 500, 0))};
    Target = DvtDrive::ResetTarget(FVector::ZeroVector, Points, Start, 20);
    TestTrue(TEXT("Nearest plus lift"), Target.GetLocation().Equals(FVector(-300, 0, 20)));
    TestTrue(TEXT("Point yaw only"), Target.Rotator().Equals(FRotator(0, 90, 0)));
    const TArray<FTransform> Vertical = { FTransform(FVector(0, 0, 800)), FTransform(FVector(200, 0, 0)) };
    TestTrue(TEXT("Distance is 3D"), DvtDrive::ResetTarget(FVector::ZeroVector, Vertical, Start, 0).GetLocation().Equals(FVector(200, 0, 0)));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtResetSequence, "DynamicVRTemplate.S1.E8E9E12.ResetSequence", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtResetSequence::RunTest(const FString&)
{
    DvtDrive::FResetSequence State;
    TestTrue(TEXT("Request accepted"), State.Request(0.3f, 0.4f));
    TestTrue(TEXT("Fading out"), State.Phase == DvtDrive::EResetPhase::FadingOut);
    TestFalse(TEXT("Out repeat ignored"), State.Request(0.3f, 0.4f));
    TestFalse(TEXT("No early move"), State.Advance(0.29f));
    int Moves = State.Advance(0.02f) ? 1 : 0;
    TestTrue(TEXT("Fading in"), State.Phase == DvtDrive::EResetPhase::FadingIn);
    TestFalse(TEXT("In repeat ignored"), State.Request(0.3f, 0.4f));
    Moves += State.Advance(0.4f) ? 1 : 0;
    TestEqual(TEXT("Exactly one move"), Moves, 1);
    TestTrue(TEXT("Idle again"), State.Phase == DvtDrive::EResetPhase::Idle);
    State.Request(0, 0.4f);
    TestTrue(TEXT("Zero out still moves"), State.Advance(0));
    TestTrue(TEXT("Zero out enters in"), State.Phase == DvtDrive::EResetPhase::FadingIn);
    State.Advance(0.4f);
    State.Request(0, 0);
    TestTrue(TEXT("Both zero moves"), State.Advance(0));
    TestTrue(TEXT("Both zero idle same frame"), State.Phase == DvtDrive::EResetPhase::Idle);
    TestFalse(TEXT("No duplicate move"), State.Advance(1));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtVehicleCount, "DynamicVRTemplate.S1.E10.VehicleCount", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtVehicleCount::RunTest(const FString&)
{
    TestFalse(TEXT("Zero invalid"), DvtDrive::HasSingleVehicle(0));
    TestTrue(TEXT("One valid"), DvtDrive::HasSingleVehicle(1));
    TestFalse(TEXT("Two invalid"), DvtDrive::HasSingleVehicle(2));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtScreenMessages, "DynamicVRTemplate.S1.E11.ScreenMessages", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtScreenMessages::RunTest(const FString&)
{
    const bool Original = GAreScreenMessagesEnabled;
    for (EWorldType::Type Type : {EWorldType::Game, EWorldType::PIE})
    {
        for (bool Initial : {true, false})
        {
            GAreScreenMessagesEnabled = Initial;
            {
                FDvtTestWorld Fixture(Type);
                if (!TestNotNull(TEXT("Game/PIE subsystem"), Fixture.World->GetSubsystem<UDvtScreenMessageSubsystem>())) { return false; }
                Fixture.World->BeginPlay();
                TestFalse(TEXT("Hidden during play"), GAreScreenMessagesEnabled);
            }
            TestEqual(TEXT("Restored"), GAreScreenMessagesEnabled, Initial);
        }
    }
    {
        FDvtTestWorld Editor(EWorldType::Editor);
        TestNull(TEXT("No editor subsystem"), Editor.World->GetSubsystem<UDvtScreenMessageSubsystem>());
    }
    GAreScreenMessagesEnabled = Original;
    return true;
}
#endif
