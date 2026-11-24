#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Spectator/DvtSpectatorModel.h"
#include "Spectator/DvtSpectatorSubsystem.h"
#include "UI/DvtUiStyle.h"
#include "Cars/DvtCarBody.h"
#include "Cars/DvtHoodVariantComponent.h"
#include "Drive/DvtVehicle.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "UObject/UnrealType.h"

struct FDvtSpectatorTestAccess
{
    static bool Supports(const UDvtSpectatorSubsystem* S, EWorldType::Type Type) { return S->DoesSupportWorldType(Type); }
    static bool Empty(const UDvtSpectatorSubsystem* S) { return !S->RenderTarget && !S->WidgetRenderer && !S->SlateWidget; }
    static TSharedRef<SWidget> Widget(UDvtSpectatorSubsystem* S, const FText& Name, bool bCar)
    {
        S->ScreenLayout.Update({1440, 1080}, S->EyeAspect, S->MinPanelWidth);
        S->CurrentPanel = {Name, Name, true};
        return S->MakeNameWidget(bCar);
    }
    static TSharedRef<SWidget> Panel(UDvtSpectatorSubsystem* S) { return S->MakeWidget(); }
};

namespace
{
    struct FSpectatorWorld
    {
        UWorld* World;
        FSpectatorWorld()
        {
            const auto Values = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(true)
                .ShouldSimulatePhysics(false).CreateNavigation(false).CreateAISystem(false);
            World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Values);
            GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
            World->InitializeActorsForPlay(FURL());
        }
        ADvtCarBody* Car(ADvtVehicle* Vehicle, const TCHAR* Name, const TArray<FString>& Hoods)
        {
            auto* Car = World->SpawnActor<ADvtCarBody>();
            Car->AttachToActor(Vehicle, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            Car->DisplayName = FText::FromString(Name);
            for (int32 Index = 0; Index < Hoods.Num(); ++Index)
            {
                auto* Hood = NewObject<UDvtHoodVariantComponent>(Car);
                Car->AddInstanceComponent(Hood);
                Hood->SetupAttachment(Car->GetRootComponent());
                Hood->SortOrder = Index;
                Hood->DisplayName = FText::FromString(Hoods[Index]);
                Hood->RegisterComponent();
            }
            Vehicle->Cars.Add(Car);
            return Car;
        }
        void BeginPlay()
        {
            World->BeginPlay();
            World->GetWorldSettings()->NotifyBeginPlay();
        }
        ~FSpectatorWorld()
        {
            World->DestroyWorld(false);
            GEngine->DestroyWorldContext(World);
        }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtSpectatorLayoutTest, "DynamicVRTemplate.S5.E1E2E3.Layout", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtSpectatorLayoutTest::RunTest(const FString&)
{
    const FIntPoint Sizes[] = {{1920,1080}, {1920,1200}, {1440,1080}, {2520,1080}};
    const double Widths[] = {0.75, 0.76875, 0.7225, 0.571429};
    const double Tops[] = {0, 0.03875, 0.13875, 0};
    DvtSpectator::FLayout L;
    for (int32 I = 0; I < UE_ARRAY_COUNT(Sizes); ++I)
    {
        TestTrue(TEXT("Changed size updates"), L.Update(Sizes[I], 4.f/3, 0.37f));
        TestTrue(TEXT("Engine validates layout"), L.Layout.IsValid());
        TestTrue(TEXT("Eye width"), FMath::IsNearlyEqual(L.Layout.EyeRectMax.X, Widths[I], 0.000001));
        TestEqual(TEXT("Eye starts left"), L.Layout.EyeRectMin.X, 0.0);
        TestTrue(TEXT("Eye top"), FMath::IsNearlyEqual(L.Layout.EyeRectMin.Y, Tops[I], 0.000001));
        TestTrue(TEXT("Eye centered vertically"), FMath::IsNearlyEqual(L.Layout.EyeRectMax.Y, 1-Tops[I], 0.000001));
        const FVector2D EyePixels = (L.Layout.EyeRectMax-L.Layout.EyeRectMin)*FVector2D(Sizes[I]);
        TestTrue(TEXT("Eye pixels preserve 4:3"), FMath::IsNearlyEqual(EyePixels.X/EyePixels.Y, 4.0/3, 0.000001));
        TestEqual(TEXT("Panel starts after eye"), L.Layout.TextureRectMin, FVector2D(L.Layout.EyeRectMax.X,0));
        TestEqual(TEXT("Panel fills remaining window"), L.Layout.TextureRectMax, FVector2D(1,1));
        TestFalse(TEXT("Panel first avoids black eye"), L.Layout.bDrawEyeFirst);
        TestTrue(TEXT("Outside eye cleared"), L.Layout.bClearBlack);
        TestFalse(TEXT("Opaque panel"), L.Layout.bUseAlpha);
        TestFalse(TEXT("Unchanged size ignored"), L.Update(Sizes[I], 4.f/3, 0.37f));
        TestFalse(TEXT("Minimized ignored"), L.Update({0,0}, 4.f/3, 0.37f));
        TestFalse(TEXT("Minimize keeps previous layout"), L.Update(Sizes[I], 4.f/3, 0.37f));
    }
    L.Update({1920,1080}, 4.f/3, 0.37f);
    TestEqual(TEXT("1080 panel pixels"), L.PanelPixels, FIntPoint(480,1080));
    TestEqual(TEXT("1080 scale"), L.DrawScale, 1.f);
    L.Update({3840,2160}, 4.f/3, 0.37f);
    TestEqual(TEXT("2160 panel pixels"), L.PanelPixels, FIntPoint(960,2160));
    TestEqual(TEXT("2160 scale"), L.DrawScale, 2.f);
    TestTrue(TEXT("Config aspect change updates"), L.Update({3840,2160}, 1.5f, 0.37f));
    TestTrue(TEXT("Config panel change updates"), L.Update({3840,2160}, 1.5f, 0.4f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtSpectatorStartTest, "DynamicVRTemplate.S5.E4.StartConditions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtSpectatorStartTest::RunTest(const FString&)
{
    FSpectatorWorld F;
    TestNull(TEXT("No vehicles"), DvtSpectator::FindVehicle(F.World));
    auto* Vehicle = F.World->SpawnActor<ADvtVehicle>();
    TestNull(TEXT("Empty Cars"), DvtSpectator::FindVehicle(F.World));
    F.Car(Vehicle, TEXT("Sample A"), {});
    F.Car(Vehicle, TEXT("Sample B"), {});
    TestNull(TEXT("Selection not ready"), DvtSpectator::FindVehicle(F.World));
    F.BeginPlay();
    TestEqual(TEXT("One ready vehicle"), DvtSpectator::FindVehicle(F.World), Vehicle);
    auto* Second = F.World->SpawnActor<ADvtVehicle>();
    TestNull(TEXT("Two vehicles rejected"), DvtSpectator::FindVehicle(F.World));
    Second->Destroy();
    TestEqual(TEXT("One remaining vehicle"), DvtSpectator::FindVehicle(F.World), Vehicle);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtSpectatorNamesTest, "DynamicVRTemplate.S5.E6E7E8E9.Names", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtSpectatorNamesTest::RunTest(const FString&)
{
    FSpectatorWorld F;
    auto* Vehicle = F.World->SpawnActor<ADvtVehicle>();
    F.Car(Vehicle, TEXT("Sample A"), {TEXT("Low"),TEXT("High")});
    F.Car(Vehicle, TEXT(""), {TEXT(""),TEXT("Raised")});
    F.Car(Vehicle, TEXT("Sample C"), {});
    F.BeginPlay();
    Vehicle->SelectHood(1);
    auto Panel = DvtSpectator::ReadPanel(*Vehicle);
    TestEqual(TEXT("Car name only"), Panel.Car.ToString(), FString(TEXT("Sample A")));
    TestEqual(TEXT("Selected hood name only"), Panel.Hood.ToString(), FString(TEXT("High")));
    TestTrue(TEXT("Hood group visible"), Panel.bShowHood);
    Vehicle->SwitchCooldownSeconds = 0;
    Vehicle->NextHood();
    TestEqual(TEXT("Next hood refreshes"), DvtSpectator::ReadPanel(*Vehicle).Hood.ToString(), FString(TEXT("Low")));
    Vehicle->NextCar();
    Panel = DvtSpectator::ReadPanel(*Vehicle);
    TestEqual(TEXT("Empty car fallback"), Panel.Car.ToString(), FString(TEXT("Car 2")));
    TestEqual(TEXT("Empty hood fallback"), Panel.Hood.ToString(), FString(TEXT("Hood 1")));
    Vehicle->NextHood();
    TestEqual(TEXT("New car hood refreshed"), DvtSpectator::ReadPanel(*Vehicle).Hood.ToString(), FString(TEXT("Raised")));
    Vehicle->NextCar();
    Panel = DvtSpectator::ReadPanel(*Vehicle);
    TestEqual(TEXT("Next car name"), Panel.Car.ToString(), FString(TEXT("Sample C")));
    TestFalse(TEXT("No hood hides group and separator"), Panel.bShowHood);
    TestTrue(TEXT("No hood text"), Panel.Hood.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtSpectatorTextTest, "DynamicVRTemplate.S5.E10.TextFit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtSpectatorTextTest::RunTest(const FString&)
{
    TestTrue(TEXT("Slate available in nullrhi"), FSlateApplication::IsInitialized());
    auto* S = NewObject<UDvtSpectatorSubsystem>();
    const auto Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    for (bool bCar : {true,false})
    {
        const auto Font = FCoreStyle::GetDefaultFontStyle(bCar ? "Bold" : "Regular", bCar ? S->CarFontSize : S->HoodFontSize);
        const float LineHeight = Measure->GetMaxCharacterHeight(Font);
        for (int32 Length : {24,60})
        {
            const FText Name = FText::FromString(Length == 24 ? FString(TEXT("Sample car configuration")) : FString::ChrN(Length, TEXT('W')));
            auto Box = FDvtSpectatorTestAccess::Widget(S, Name, bCar);
            Box->SlatePrepass(1.f);
            auto Text = StaticCastSharedRef<STextBlock>(Box->GetChildren()->GetChildAt(0));
            TestTrue(TEXT("Name wraps in minimum panel width"), Text->GetDesiredSize().X <= 334.01f);
            TestTrue(TEXT("Name box limited to three lines"), Box->GetDesiredSize().Y <= 3*LineHeight+0.01f);
            if (Length == 24) { TestTrue(TEXT("24-character name fits three lines"), Text->GetDesiredSize().Y <= 3*LineHeight+0.01f); }
            if (Length == 60) { TestTrue(TEXT("Long name needs ellipsis"), Text->GetDesiredSize().Y > Box->GetDesiredSize().Y); }
        }
    }
    auto Panel = FDvtSpectatorTestAccess::Panel(S);
    Panel->SlatePrepass(1.f);
    TestTrue(TEXT("Both groups fit in 1080 height"), Panel->GetDesiredSize().Y <= 1080);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtSpectatorConfigTest, "DynamicVRTemplate.S5.E11E12.ConfigAndPalette", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtSpectatorConfigTest::RunTest(const FString&)
{
    auto* CDO = GetMutableDefault<UDvtSpectatorSubsystem>();
    TestEqual(TEXT("Game config"), CDO->GetClass()->ClassConfigName, FName(TEXT("Game")));
    for (const TCHAR* Name : {TEXT("EyeAspect"),TEXT("MinPanelWidth"),TEXT("RedrawsPerSecond"),TEXT("HeadingFontSize"),TEXT("CarFontSize"),TEXT("HoodFontSize"),TEXT("BackgroundColor"),TEXT("TextColor"),TEXT("SecondaryTextColor"),TEXT("LineColor")})
    {
        const auto* Property = CDO->GetClass()->FindPropertyByName(FName(Name));
        TestTrue(Name, Property && Property->HasAnyPropertyFlags(CPF_Config));
    }
    TestEqual(TEXT("Background sRGB"), CDO->BackgroundColor, DvtUiStyle::Color(TEXT("131a26")));
    TestEqual(TEXT("Text sRGB"), CDO->TextColor, DvtUiStyle::Color(TEXT("eef2f7")));
    TestEqual(TEXT("Secondary sRGB"), CDO->SecondaryTextColor, DvtUiStyle::Color(TEXT("9aa6b8")));
    TestEqual(TEXT("Line sRGB"), CDO->LineColor, DvtUiStyle::Color(TEXT("2e3a4e")));
    const float Original = CDO->EyeAspect;
    CDO->EyeAspect = 1.5f;
    {
        FSpectatorWorld F;
        TestEqual(TEXT("New world inherits CDO"), F.World->GetSubsystem<UDvtSpectatorSubsystem>()->EyeAspect, 1.5f);
    }
    CDO->EyeAspect = Original;
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDvtSpectatorInactiveTest, "DynamicVRTemplate.S5.E5E13.NoHeadsetAndWorldTypes", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDvtSpectatorInactiveTest::RunTest(const FString&)
{
    const auto* CDO = GetDefault<UDvtSpectatorSubsystem>();
    for (EWorldType::Type Type : {EWorldType::Game,EWorldType::PIE,EWorldType::Editor,EWorldType::EditorPreview})
    {
        TestEqual(TEXT("Only Game and PIE supported"), FDvtSpectatorTestAccess::Supports(CDO,Type), Type==EWorldType::Game || Type==EWorldType::PIE);
    }
    FSpectatorWorld F;
    auto* Vehicle = F.World->SpawnActor<ADvtVehicle>();
    F.Car(Vehicle,TEXT("Sample A"),{});
    F.BeginPlay();
    auto* S = F.World->GetSubsystem<UDvtSpectatorSubsystem>();
    for (int32 I=0; I<64; ++I) { S->Tick(0.5f); }
    TestTrue(TEXT("No renderer, widget or target without headset"), FDvtSpectatorTestAccess::Empty(S));
    TestFalse(TEXT("Retry finishes silently after 30 s"), S->IsTickable());
    return true;
}
#endif
