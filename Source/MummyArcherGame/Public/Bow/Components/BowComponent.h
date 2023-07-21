// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "BowComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUMMYARCHERGAME_API UBowComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UBowComponent();
	
	UFUNCTION(BlueprintCallable, Category=Weapon)
		void AttachBowToCharacter(class AMummyCharacter* TargetCharacter);

protected:
	virtual void InitializeComponent() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
		AMummyCharacter* Pawn;
	
	UPROPERTY(EditDefaultsOnly, Category=BowSettings, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class ABasicArrowProjectile> ArrowProjectileClass;
	UPROPERTY()
		const ABasicArrowProjectile* ArrowCDO;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=BowSettings, meta = (AllowPrivateAccess = "true"))
		float MaxBowTensionTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="BowSettings|Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> BowPowerWidgetClass;
	UPROPERTY()
		class UBowPowerWidget* BowPowerWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="BowSettings|Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> SightWidgetClass;
	UPROPERTY()
		UUserWidget* SightWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Bow", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* BowMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Bow", meta = (AllowPrivateAccess = "true"))
	class UInputAction* BowFireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Bow", meta = (AllowPrivateAccess = "true"))
	UInputAction* BowFocusAction;
	
private:
	float BowPowerScale;
	FVector InitialArrowDirection;
	
private:
	// Action functions
	UFUNCTION()
		void Focus(const struct FInputActionValue& Value);
	UFUNCTION()
		void FireButtonHolding(const struct FInputActionInstance& ActionInstance);
	UFUNCTION()
		void FireButtonPressed();
	UFUNCTION()
		void Fire();
	UFUNCTION()
		void FireButtonReleased();
	
	//
	FTransform CalculateArrowTransform();
	void CreateArrow(UWorld* const World);

	void TraceLine(UWorld* const World, const FVector& Start, const FVector& End, bool DrawTrace, FHitResult& HitResult);
};
