// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "UI/GameHUDWidget.h"
#include "BowComponent.generated.h"

UENUM(BlueprintType, Blueprintable)
namespace Arrow
{
	enum EType
	{
		None = 0,
		Basic,
		Teleportation
	};
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

	UPROPERTY(EditDefaultsOnly, Category=BowSettings, meta = (AllowPrivateAccess = "true"))
	TMap<TEnumAsByte<Arrow::EType>, TSubclassOf<class ABasicArrowProjectile>> ArrowTypes;

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
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	// Action functions
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
		void Fire(const FTransform& SpawnTransform);
	UFUNCTION(Server, Reliable)
		void Server_Fire(const FTransform& SpawnTransform);

	// Spline
	void ResetSpline();
	void DrawSpline(const struct FPredictProjectilePathResult& ProjectilePathResult);

public:
	UFUNCTION(BlueprintCallable)
	void ChangeArrow(Arrow::EType ArrowType);
private:
	UFUNCTION(Server, Reliable)
	void Server_ChangeArrow(TSubclassOf<ABasicArrowProjectile> InCurrentArrow, UStaticMesh* ArrowMesh);

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
	TSubclassOf<ABasicArrowProjectile> CurrentArrow;
	TObjectPtr<const ABasicArrowProjectile> ArrowCDO;

	TArray<class USplineMeshComponent*> SplineMeshes;
	
	UPROPERTY()
		TObjectPtr<UGameHUDWidget> GameHUDWidget;
	
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
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	float TensionPercent;
	
	float TimerBeforeGetArrow;
};
