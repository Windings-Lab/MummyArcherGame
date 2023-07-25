// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasicArrowProjectile.generated.h"

USTRUCT()
struct FArrowParameters
{
	GENERATED_BODY()

	FVector SpawnLocation;
	FVector ImpactPoint;
	float   Speed;
};

UCLASS(config=Game)
class ABasicArrowProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABasicArrowProjectile();

	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
	FORCEINLINE float GetMinSpeed() const { return MinSpeed; }
	FORCEINLINE float GetMaxSpeed() const { return MaxSpeed; }
	FORCEINLINE float GetGravityScale() const { return GravityScale; }
	
	void PredictArrowPath(UWorld* const World, FArrowParameters& ArrowParameters, bool DrawArc, struct FPredictProjectilePathResult& ProjectilePathResult) const;
	float CalculateArrowSpeed(float BowTensionTime, float BowMaxTensionTime) const;
	virtual void CreateArrow(UWorld* const World, FArrowParameters& ArrowParameters, const TSubclassOf<ABasicArrowProjectile>& ArrowProjectileClass) const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=ArrowSettings, meta = (AllowPrivateAccess = "true"))
	float MinSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=ArrowSettings, meta = (AllowPrivateAccess = "true"))
	float MaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=ArrowSettings, meta = (AllowPrivateAccess = "true"))
	float GravityScale;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Components, meta = (AllowPrivateAccess = "true"))
	USceneComponent* RootSceneComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Components, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Arrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Components, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxCollider;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UFUNCTION()
		void OnArrowBeginOverlap(UPrimitiveComponent* OverlappedComponent
			, AActor* OtherActor
			, UPrimitiveComponent* OtherComp
			, int32 OtherBodyIndex
			, bool bFromSweep
			, const FHitResult & SweepResult);
};

