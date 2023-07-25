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

	void SetupPlayerInput(UInputComponent* PlayerInputComponent);
	
	void AddBowMappingContext(class UEnhancedInputLocalPlayerSubsystem* Subsystem, int Priority);
	void RemoveBowMappingContext(class UEnhancedInputLocalPlayerSubsystem* Subsystem);

protected:
	virtual void InitializeComponent() override;

private:
	UPROPERTY()
		class ABasicCharacter* Pawn;
	
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
	// Action functions
	UFUNCTION()
		void Focus(const struct FInputActionValue& Value);
	UFUNCTION()
		void FireButtonHolding(const struct FInputActionInstance& ActionInstance);
	UFUNCTION()
		void FireButtonPressed();
	UFUNCTION()
		void Fire(const struct FInputActionInstance& ActionInstance);
	UFUNCTION()
		void FireButtonReleased();
};
