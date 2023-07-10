// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "WeaponComponent.generated.h"

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
class MUMMYARCHERGAME_API UWeaponComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

	UWeaponComponent();

public:
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Weapon)
	TSubclassOf<class AArrowProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> BowPowerWidgetClass;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Weapon)
	FVector MuzzleOffset;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	class UInputMappingContext* FireMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Input)
	FFireAction FireActionStruct;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Weapon)
	float ArrowMaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Weapon)
	float ArrowMinSpeed;

	UFUNCTION(BlueprintCallable, Category=Input)
	void FireButtonPresses(const FInputActionInstance& ActionInstance);
	
	UFUNCTION(BlueprintCallable, Category=Input)
	void Fire(const FInputActionInstance& ActionInstance);

	UFUNCTION(BlueprintCallable, Category=Input)
	void FireButtonReleased(const FInputActionInstance& ActionInstance);
	
	UFUNCTION(BlueprintCallable, Category=Input)
	void CalculateArrowPath(const FInputActionInstance& ActionInstance);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Input)
	FFocusAction FocusActionStruct;
	
	UFUNCTION(BlueprintCallable, Category=Input)
	void Focus(const FInputActionValue& Value);

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category=Weapon)
	void AttachWeapon(ABasicCharacter* TargetCharacter);

protected:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;
	
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	class ABasicCharacter* Character;
	
	UPROPERTY()
	UUserWidget* SightWidget;

	UPROPERTY()
	class UBowPowerWidget* BowPowerWidget;

	float CalculateArrowSpeed(float MinPower, float MaxPower, float ElapsedTime) const;

	float ForceScale;
};
