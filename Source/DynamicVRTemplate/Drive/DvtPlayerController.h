#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Drive/DvtDriveLogic.h"
#include "DvtPlayerController.generated.h"

class ADvtVehicle;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class DYNAMICVRTEMPLATE_API ADvtPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ADvtPlayerController();
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category="Drive")
    void ResetVehicle();

    UPROPERTY(EditDefaultsOnly, Category="Drive|Input")
    TObjectPtr<UInputMappingContext> DriveMappingContext;
    UPROPERTY(EditDefaultsOnly, Category="Drive|Input")
    TObjectPtr<UInputAction> SteerAction;
    UPROPERTY(EditDefaultsOnly, Category="Drive|Input")
    TObjectPtr<UInputAction> ThrottleAction;
    UPROPERTY(EditDefaultsOnly, Category="Drive|Input")
    TObjectPtr<UInputAction> BrakeAction;
    UPROPERTY(EditDefaultsOnly, Category="Drive|Input")
    TObjectPtr<UInputAction> ResetAction;
    UPROPERTY(EditDefaultsOnly, Category="Drive|Input")
    TObjectPtr<UInputAction> PreviousCarAction;
    UPROPERTY(EditDefaultsOnly, Category="Drive|Input")
    TObjectPtr<UInputAction> NextCarAction;
    UPROPERTY(EditDefaultsOnly, Category="Drive|Input")
    TObjectPtr<UInputAction> PreviousHoodAction;
    UPROPERTY(EditDefaultsOnly, Category="Drive|Input")
    TObjectPtr<UInputAction> NextHoodAction;
    UPROPERTY(EditAnywhere, Category="Drive|Reset", meta=(Units="cm", ClampMin="0"))
    float ResetLiftCm = 20;
    UPROPERTY(EditAnywhere, Category="Drive|Reset", meta=(Units="s", ClampMin="0"))
    float ResetFadeOutSeconds = 0.3f;
    UPROPERTY(EditAnywhere, Category="Drive|Reset", meta=(Units="s", ClampMin="0"))
    float ResetFadeInSeconds = 0.4f;

private:
    friend class FDvtInputs;
#if WITH_DEV_AUTOMATION_TESTS
    void SetDriveVehicleForTest(ADvtVehicle* InVehicle) { Vehicle = InVehicle; }
#endif
    void AttachVRPawn();
    void ConnectTrackedHands();
    void Steer(const FInputActionValue& Value);
    void Throttle(const FInputActionValue& Value);
    void Brake(const FInputActionValue& Value);
    void SteerCompleted(const FInputActionValue& Value);
    void ThrottleCompleted(const FInputActionValue& Value);
    void BrakeCompleted(const FInputActionValue& Value);
    void AdvanceReset(float DeltaSeconds);
    void PreviousCar();
    void NextCar();
    void PreviousHood();
    void NextHood();

    UPROPERTY(Transient)
    TObjectPtr<ADvtVehicle> Vehicle;
    DvtDrive::FResetSequence ResetSequence;
    FTransform PendingResetTarget;
};
