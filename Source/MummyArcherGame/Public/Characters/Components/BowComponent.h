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

	// Change Arrows -------------------------
	// * Change Arrow State
public:
	UFUNCTION(BlueprintCallable)
	void ChangeArrow(Arrow::EType ArrowType);
private:
	UFUNCTION(Server, Reliable)
	void Server_ChangeArrow(TSubclassOf<ABasicArrowProjectile> InCurrentArrow, UStaticMesh* ArrowMesh);

	// * Change Arrow State - GetArrowFromQuiver notification
	UFUNCTION(BlueprintCallable)
	void OnGetArrowFromQuiver();
	UFUNCTION(Server, Reliable)
	void Server_OnGetArrowFromQuiver();

	// * Transition to FocusIdle state
	UFUNCTION(BlueprintCallable)
	void OnChangeArrowFinished();
	UFUNCTION(Server, Reliable)
	void Server_OnChangeArrowFinished();
	// Change Arrows -------------------------
	
	UFUNCTION(BlueprintCallable)
	void OnInterrupted();
	UFUNCTION(Server, Reliable)
	void Server_OnInterrupted();

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
	bool bFocusIdle;
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	bool bBowTensionIdle;
	UPROPERTY(Replicated, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	bool bChangeArrow;
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	float TensionPercent;
	
	float TimerBeforeGetArrow;
};
