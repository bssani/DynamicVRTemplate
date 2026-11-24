#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Grip/DvtGripLogic.h"
#include "Grip/DvtSteeringGripComponent.h"
#include "Tests/DvtGripTestReceiver.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtRimDistance, "DynamicVRTemplate.S2.E1E2E3.RimDistance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtRimDistance::RunTest(const FString&)
{
    TestEqual(TEXT("On rim"), DvtGrip::DistanceToRim(FVector(0, 18, 0), 18), 0.f);
    TestEqual(TEXT("Hub"), DvtGrip::DistanceToRim(FVector::ZeroVector, 18), 18.f);
    TestEqual(TEXT("Axial and radial distance"), DvtGrip::DistanceToRim(FVector(5, 30, 0), 18), 13.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtGripEnter, "DynamicVRTemplate.S2.E4.EnterBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtGripEnter::RunTest(const FString&)
{
    TestTrue(TEXT("Inside enters"), DvtGrip::NextGrip(false, true, true, 9.9f, 10, 18));
    TestFalse(TEXT("Equal stays released"), DvtGrip::NextGrip(false, true, true, 10, 10, 18));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtGripRelease, "DynamicVRTemplate.S2.E5.ReleaseBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtGripRelease::RunTest(const FString&)
{
    TestTrue(TEXT("Equal stays gripped"), DvtGrip::NextGrip(true, true, true, 18, 10, 18));
    TestFalse(TEXT("Outside releases"), DvtGrip::NextGrip(true, true, true, 18.1f, 10, 18));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtGripHysteresis, "DynamicVRTemplate.S2.E6.Hysteresis", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtGripHysteresis::RunTest(const FString&)
{
    TestTrue(TEXT("Gripped in band"), DvtGrip::NextGrip(true, true, true, 15, 10, 18));
    TestFalse(TEXT("Released in band"), DvtGrip::NextGrip(false, true, true, 15, 10, 18));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtGripLostTracking, "DynamicVRTemplate.S2.E7.LostTracking", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtGripLostTracking::RunTest(const FString&)
{
    TestTrue(TEXT("Hold grip through occlusion"), DvtGrip::NextGrip(true, true, false, 100, 10, 18));
    TestFalse(TEXT("Keep released through occlusion"), DvtGrip::NextGrip(false, true, false, 0, 10, 18));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtGripUnavailable, "DynamicVRTemplate.S2.E8.UnavailableTracker", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtGripUnavailable::RunTest(const FString&)
{
    for (bool Gripped : {true, false})
    {
        TestTrue(TEXT("No valid tracking assumes grip"), DvtGrip::NextGrip(Gripped, false, false, 100, 10, 18));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtGripThresholdOrder, "DynamicVRTemplate.S2.E9.ThresholdOrder", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtGripThresholdOrder::RunTest(const FString&)
{
    TestTrue(TEXT("Release clamped to enter at equality"), DvtGrip::NextGrip(true, true, true, 10, 10, 5));
    TestFalse(TEXT("Release clamped to enter outside"), DvtGrip::NextGrip(true, true, true, 10.1f, 10, 5));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtGripVisibility, "DynamicVRTemplate.S2.E10.Visibility", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtGripVisibility::RunTest(const FString&)
{
    for (bool Tracked : {true, false})
    {
        TestFalse(TEXT("Grip hides tracked"), DvtGrip::Visibility(true, Tracked).bTracked);
        TestTrue(TEXT("Grip shows fixed pose"), DvtGrip::Visibility(true, Tracked).bGrip);
    }
    TestTrue(TEXT("Released tracked shows tracked"), DvtGrip::Visibility(false, true).bTracked);
    TestFalse(TEXT("Released tracked hides fixed pose"), DvtGrip::Visibility(false, true).bGrip);
    TestFalse(TEXT("Released lost hides tracked"), DvtGrip::Visibility(false, false).bTracked);
    TestFalse(TEXT("Released lost hides fixed pose"), DvtGrip::Visibility(false, false).bGrip);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtGripDefaults, "DynamicVRTemplate.S2.E11.Defaults", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtGripDefaults::RunTest(const FString&)
{
    const auto* Grip = GetDefault<UDvtSteeringGripComponent>();
    TestTrue(TEXT("Hidden in game"), Grip->bHiddenInGame);
    TestTrue(TEXT("No collision"), Grip->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
    TestEqual(TEXT("Rim cm"), Grip->RimRadiusCm, 18.f);
    TestEqual(TEXT("Enter cm"), Grip->GripEnterDistanceCm, 10.f);
    TestEqual(TEXT("Release cm"), Grip->GripReleaseDistanceCm, 18.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtGripIndependentEvent, "DynamicVRTemplate.S2.E12.IndependentEvent", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtGripIndependentEvent::RunTest(const FString&)
{
    auto* Grip = NewObject<UDvtSteeringGripComponent>();
    auto* Receiver = NewObject<UDvtGripTestReceiver>();
    Grip->OnGripChanged.AddDynamic(Receiver, &UDvtGripTestReceiver::Record);
    TestTrue(TEXT("Left starts gripped"), Grip->GetGrip(EControllerHand::Left) == EDvtHandGrip::Gripped);
    TestTrue(TEXT("Right starts gripped"), Grip->GetGrip(EControllerHand::Right) == EDvtHandGrip::Gripped);
    Grip->ApplyHandSample(EControllerHand::Left, true, true, 19);
    Grip->ApplyHandSample(EControllerHand::Right, true, true, 0);
    Grip->ApplyHandSample(EControllerHand::Left, true, true, 19);
    TestEqual(TEXT("Exactly one event"), Receiver->Count, 1);
    TestTrue(TEXT("Only left event"), Receiver->LastHand == EControllerHand::Left);
    TestTrue(TEXT("Released event"), Receiver->LastState == EDvtHandGrip::Released);
    TestTrue(TEXT("Right unchanged"), Grip->GetGrip(EControllerHand::Right) == EDvtHandGrip::Gripped);
    return true;
}
#endif
