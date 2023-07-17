// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "BowComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MUMMYARCHERGAME_API UBowComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

	UBowComponent();

public:
	UFUNCTION(BlueprintCallable, Category=Weapon)
	void AttachBowToCharacter(class AMummyCharacter* TargetCharacter);

public:
	UPROPERTY(EditDefaultsOnly, Category=BowSettings)
	TSubclassOf<class AArrowProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=BowSettings)
	float ArrowMaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=BowSettings)
	float ArrowMinSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=BowSettings)
	float TimeUntilMaxPower;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=BowSettings)
	float ArrowGravityScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="BowSettings|Widgets")
	TSubclassOf<UUserWidget> BowPowerWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="BowSettings|Widgets")
	TSubclassOf<UUserWidget> SightWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	class UInputMappingContext* FireMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	class UInputAction* BowFireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Input)
	class UInputAction* BowFocusAction;

protected:

	virtual void BeginPlay() override;
	
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Action functions
	UFUNCTION()
	void Focus(const struct FInputActionValue& Value);
	
	UFUNCTION()
	void FireButtonHolding(const struct FInputActionInstance& ActionInstance);
	
	UFUNCTION()
	void FireButtonPresses(const FInputActionInstance& ActionInstance);
	
	UFUNCTION()
	void Fire(const FInputActionInstance& ActionInstance);

	UFUNCTION()
	void FireButtonReleased(const FInputActionInstance& ActionInstance);

	//
	FTransform CalculateArrowTransform();
	float CalculateArrowSpeed(float MinPower, float MaxPower, float HoldTime) const;

	void PredictArrowPath(UWorld* const World, struct FPredictProjectilePathResult& ProjectilePathResult);
	void CreateArrow(UWorld* const World);

	void BowTraceLine(UWorld* const World, double Distance, bool DrawTrace, FHitResult& HitResult);

private:
	UPROPERTY()
	class AMummyCharacter* Character;
	
	UPROPERTY()
	UUserWidget* SightWidget;
	UPROPERTY()
	class UBowPowerWidget* BowPowerWidget;
	
	float PowerScale;
	
	FVector InitialArrowDirection;
	FVector ImpactPoint;
};
