#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Cars/DvtSelectionLogic.h"
#include "DvtVehicle.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UDvtSteeringGripComponent;
class USkeletalMeshComponent;
class ADvtCarBody;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDvtSelectionChanged, int32, CarIndex, int32, HoodIndex);

UCLASS()
class DYNAMICVRTEMPLATE_API ADvtVehicle : public AWheeledVehiclePawn
{
    GENERATED_BODY()
public:
    ADvtVehicle();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void ApplySteering(float Value);
    void TeleportForReset(const FTransform& Target);

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Cars")
    TArray<TObjectPtr<ADvtCarBody>> Cars;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars")
    int32 InitialCar = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars", meta=(Units="s", ClampMin="0"))
    float SwitchCooldownSeconds = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cars")
    bool bHideChassisWhenCarsExist = true;
    UPROPERTY(BlueprintAssignable, Category="Cars")
    FDvtSelectionChanged OnSelectionChanged;

    UFUNCTION(BlueprintCallable, Category="Cars")
    void SelectCar(int32 Index);
    UFUNCTION(BlueprintCallable, Category="Cars")
    void NextCar();
    UFUNCTION(BlueprintCallable, Category="Cars")
    void PreviousCar();
    UFUNCTION(BlueprintCallable, Category="Cars")
    void SelectHood(int32 Index);
    UFUNCTION(BlueprintCallable, Category="Cars")
    void NextHood();
    UFUNCTION(BlueprintCallable, Category="Cars")
    void PreviousHood();
    UFUNCTION(BlueprintPure, Category="Cars")
    int32 GetCarIndex() const { return Selection.CarIndex; }
    UFUNCTION(BlueprintPure, Category="Cars")
    int32 GetCarCount() const { return Cars.Num(); }
    UFUNCTION(BlueprintPure, Category="Cars")
    FText GetCarName() const;
    UFUNCTION(BlueprintPure, Category="Cars")
    int32 GetHoodIndex() const { return Selection.HoodIndex; }
    UFUNCTION(BlueprintPure, Category="Cars")
    int32 GetHoodCount() const { return Selection.HoodCount(); }
    UFUNCTION(BlueprintPure, Category="Cars")
    FText GetHoodName() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drive")
    TObjectPtr<USceneComponent> DriverOrigin;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drive")
    TObjectPtr<USceneComponent> SteeringWheelPivot;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Drive")
    TObjectPtr<UStaticMeshComponent> SteeringWheelMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VR|Grip")
    TObjectPtr<UDvtSteeringGripComponent> SteeringGrip;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VR|Grip")
    TObjectPtr<USkeletalMeshComponent> LeftGripHand;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VR|Grip")
    TObjectPtr<USkeletalMeshComponent> RightGripHand;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drive", meta=(Units="deg", ClampMin="0"))
    float SteeringLockToLockDegrees = 900;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drive")
    TEnumAsByte<EAxis::Type> SteeringSpinAxis = EAxis::X;

    const FTransform& GetStartTransform() const { return StartTransform; }

private:
    void InitializeCars();
    void ApplySelection(bool bCarChanged, bool bNotify);
    void StepCar(int32 Direction);
    void StepHood(int32 Direction);
    double ReadSwitchTimeSeconds() const;
    ADvtCarBody* GetSelectedCar() const;
    DvtSelection::FSelection Selection;
    struct FBodyVisibility
    {
        TWeakObjectPtr<UPrimitiveComponent> Component;
        bool bHiddenInGame;
    };
    TArray<FBodyVisibility> OriginalBodyVisibility;
    FTransform StartTransform;
    FQuat InitialWheelRotation = FQuat::Identity;
};
