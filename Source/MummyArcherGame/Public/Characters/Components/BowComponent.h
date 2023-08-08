// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "UI/GameHUDWidget.h"
#include "BowComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUMMYARCHERGAME_API UBowComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category=BowSettings, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ABasicArrowProjectile> ArrowProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="BowSettings", meta = (AllowPrivateAccess = "true"))
	class UProjectilePathPredictor* ArrowPathPredictor;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="BowSettings", meta = (AllowPrivateAccess = "true"))
	float MaxBowTensionTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Bow", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* BowMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Bow", meta = (AllowPrivateAccess = "true"))
	class UInputAction* BowFireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Bow", meta = (AllowPrivateAccess = "true"))
	UInputAction* BowFocusAction;
	
public:
	UBowComponent();

	void SetupPlayerInput(UInputComponent* PlayerInputComponent);
	
	void AddBowMappingContext(class UEnhancedInputLocalPlayerSubsystem* Subsystem, int Priority);
	void RemoveBowMappingContext(class UEnhancedInputLocalPlayerSubsystem* Subsystem);

protected:
	virtual void InitializeComponent() override;
	
private:
	// Action functions
	UFUNCTION()
		void Focus(const struct FInputActionValue& Value);
	UFUNCTION()
		void FireButtonHolding(const struct FInputActionInstance& ActionInstance);
	UFUNCTION()
		void FireButtonPressed();

		struct FProjectileParams CreateArrowParams(float BowTensionTime);
	UFUNCTION()
		void Fire(const struct FInputActionInstance& ActionInstance);
		void Fire(const FTransform& SpawnTransform);
	UFUNCTION(Server, Reliable)
		void Server_Fire(const FTransform& SpawnTransform);
	
	UFUNCTION()
		void FireButtonReleased();

private:
	class ABasicCharacter* Pawn;
	
	const ABasicArrowProjectile* ArrowCDO;
	
	UPROPERTY()
		TObjectPtr<UGameHUDWidget> GameHUDWidget;

	// Animation Property
	float CurrentTensionTime;
};
