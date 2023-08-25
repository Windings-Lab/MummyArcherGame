// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "BowComponent.generated.h"

namespace Arrow
{
	enum EType : int;
}

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUMMYARCHERGAME_API UBowComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BowSettings|Components", meta=(AllowPrivateAccess = "true"))
	class USplineComponent* ArrowSplinePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BowSettings|Components", meta=(AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ArcEndSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="BowSettings", meta=(AllowPrivateAccess = "true"))
	UStaticMesh* ArcSplineMesh;

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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animation", meta = (AllowPrivateAccess = "true"))
	FRotator OnTensionOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animation", meta = (AllowPrivateAccess = "true"))
	FRotator OnFirePressedOffset;
	
public:
	UBowComponent();

	void SetupPlayerInput(UInputComponent* PlayerInputComponent);
	
	void AddBowMappingContext(class UEnhancedInputLocalPlayerSubsystem* Subsystem, int Priority);
	void RemoveBowMappingContext(class UEnhancedInputLocalPlayerSubsystem* Subsystem);

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	// Enchanted Input Action functions
	UFUNCTION()
		void FocusAction(const struct FInputActionValue& Value);
		void Focus();
		void Unfocus();
	UFUNCTION(Server, Reliable)
		void Server_Focus(bool InFocused, bool InBowTensionIdle);
	
	UFUNCTION()
		void FireButtonHolding(const struct FInputActionInstance& ActionInstance);
	UFUNCTION(Server, Reliable)
		void Server_FireButtonHolding(float InTensionPercent);
	UFUNCTION()
		void FireButtonPressed();
	UFUNCTION(Server, Reliable)
		void Server_FireButtonPressed();
	UFUNCTION()
		void FireButtonReleased();
	UFUNCTION(Server, Reliable)
		void Server_FireButtonReleased(bool InBowTensionIdle);

		struct FProjectileParams CreateArrowParams(float BowTensionTime);
	UFUNCTION()
		void Fire(const struct FInputActionInstance& ActionInstance);
		void Fire(const FTransform& InTransform, float Speed);
	UFUNCTION(Server, Reliable)
		void Server_Fire(const FTransform& InTransform, float Speed);

	// Spline
	void ResetSpline();
	void DrawSpline(const struct FPredictProjectilePathResult& ProjectilePathResult);

	// Basic Input Events --------------------------------------
public:
	UFUNCTION(BlueprintCallable)
	void OnFirePressed();
	UFUNCTION(BlueprintCallable)
	void OnFireReleased();
	// Basic Input Events --------------------------------------
	
public:
	void SetArrow(TSubclassOf<class ABasicArrowProjectile> InCurrentArrow);
private:
	UFUNCTION(Server, Reliable)
	void Server_SetArrow(TSubclassOf<class ABasicArrowProjectile> InCurrentArrow);

public:
	void SetArrowType(TEnumAsByte<Arrow::EType> ArrowType);
private:
	UFUNCTION(Server, Reliable)
	void Server_SetArrowType(Arrow::EType ArrowType);

	// Animation States -------------------------
	// * (ChangeArrow State) || (FocusIdle State) -> Idle State
	UFUNCTION(BlueprintCallable)
	void OnReturnToIdleState();
	
	// * Change Arrow State - GetArrowFromQuiver notification
	UFUNCTION(BlueprintCallable)
	void OnGetArrowFromQuiver();

	// * ChangeArrow State -> FocusIdle State
	UFUNCTION(BlueprintCallable)
	void OnChangeArrowFinished();

	// * FocusIdle State -> BowTensionIdle State
	UFUNCTION(BlueprintCallable)
	void OnBowTensionIdleState(bool InState);
	
	UFUNCTION(BlueprintCallable)
	void OnInterrupted();
	// Animation States -------------------------

private:
	TObjectPtr<class AMummyCharacter> Pawn;

	UPROPERTY(Replicated)
	TEnumAsByte<Arrow::EType> CurrentArrowType;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentArrow)
	class ABasicArrowProjectile* CurrentArrow;

	UFUNCTION()
	void OnRep_CurrentArrow();

	TArray<class USplineMeshComponent*> SplineMeshes;
	
	UPROPERTY()
		TObjectPtr<class UGameHUDWidget> GameHUDWidget;
	
	// Animation Property
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	bool bFocused;
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	bool bTransitionToFocusIdle;
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	bool bTransitionToBowTensionIdle;
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	bool bInBowTensionIdleState;
	
	UPROPERTY(Replicated, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	bool bTransitionToChangeArrow;
	bool bChangingArrow;
	
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	float TensionPercent;

	float TimerOnFirePressed;
	float TimerBeforeGetArrow;
};
