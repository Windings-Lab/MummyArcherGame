// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "BowComponent.generated.h"

USTRUCT(BlueprintType)
struct FFireAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UInputAction* FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TimeUntilMaxPower;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TimeToForceShoot;
};

USTRUCT(BlueprintType)
struct FFocusAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UInputAction* FocusAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> SightWidgetClass;
};

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MUMMYARCHERGAME_API UBowComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

	UBowComponent();

public:
	UFUNCTION(BlueprintCallable, Category=Weapon)
	void AttachWeapon(ACharacter* TargetCharacter);

public:
	UPROPERTY(EditDefaultsOnly, Category=Weapon)
	TSubclassOf<class AArrowProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> BowPowerWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	class UInputMappingContext* FireMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Input)
	FFireAction FireActionStruct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Weapon)
	float ArrowMaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Weapon)
	float ArrowMinSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Input)
	FFocusAction FocusActionStruct;

protected:

	virtual void BeginPlay() override;
	
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Action functions
	UFUNCTION(BlueprintCallable, Category=Input)
	void FireButtonHolding(const FInputActionInstance& ActionInstance);
	
	UFUNCTION(BlueprintCallable, Category=Input)
	void FireButtonPresses(const FInputActionInstance& ActionInstance);
	
	UFUNCTION(BlueprintCallable, Category=Input)
	void Fire(const FInputActionInstance& ActionInstance);

	UFUNCTION(BlueprintCallable, Category=Input)
	void FireButtonReleased(const FInputActionInstance& ActionInstance);
	
	UFUNCTION(BlueprintCallable, Category=Input)
	void Focus(const FInputActionValue& Value);

	//
	float CalculateArrowSpeed(float MinPower, float MaxPower, float HoldTime) const;
	FTransform CalculateArrowTransform() const;
	void CreateArrow(UWorld* const World, float HoldTime);
	void BowTraceLine(UWorld* const World);

private:
	UPROPERTY()
	class ACharacter* Character;
	
	UPROPERTY()
	UUserWidget* SightWidget;
	UPROPERTY()
	class UBowPowerWidget* BowPowerWidget;
	
	float PowerScale;

	FVector CrosshairLocation;
	FVector ImpactPoint;
};
