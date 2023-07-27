// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasicArrowProjectile.generated.h"

USTRUCT()
struct FArrowParameters
{
	GENERATED_BODY()

	FArrowParameters() : Class(nullptr), Speed(0.f) {}
	FArrowParameters(UClass* const InClass, const FTransform& InSpawnTransform, const FVector& InImpactPoint, float InSpeed)
		: Class(InClass), SpawnTransform(InSpawnTransform.GetRotation(), InSpawnTransform.GetLocation(), FVector::One())
			, ImpactPoint(InImpactPoint) , Speed(InSpeed)
	{}

	FArrowParameters(UClass* const InClass, const FVector& InLocation, const FVector& InImpactPoint, float InSpeed)
	: Class(InClass), SpawnTransform(FQuat(), InLocation, FVector::One())
		, ImpactPoint(InImpactPoint) , Speed(InSpeed)
	{}
	
	UClass* Class;
	FTransform SpawnTransform;
	FVector ImpactPoint;
	float   Speed;
};

UCLASS(config=Game)
class ABasicArrowProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Components, meta = (AllowPrivateAccess = "true"))
	USceneComponent* RootSceneComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Components, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Arrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Components, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxCollider;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

public:
	ABasicArrowProjectile();

	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
	FORCEINLINE float GetMinSpeed() const { return MinSpeed; }
	FORCEINLINE float GetMaxSpeed() const { return MaxSpeed; }
	FORCEINLINE float GetGravityScale() const { return GravityScale; }
	
	void PredictArrowPath(UWorld* const World, FArrowParameters& ArrowParameters, bool DrawArc, struct FPredictProjectilePathResult& ProjectilePathResult) const;
	float CalculateArrowSpeed(float BowTensionTime, float BowMaxTensionTime) const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Called when client arrow still not collided with something, when server arrow is collided
	virtual void OnRep_AttachmentReplication() override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=ArrowSettings, meta = (AllowPrivateAccess = "true"))
	float MinSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=ArrowSettings, meta = (AllowPrivateAccess = "true"))
	float MaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=ArrowSettings, meta = (AllowPrivateAccess = "true"))
	float GravityScale;

private:
	UPROPERTY(Replicated)
	FTransform Server_RootRelativeTransform;

	UPROPERTY(Replicated)
	FTransform Server_ArrowRelativeTransform;

private:
	UFUNCTION()
		void OnArrowBeginOverlap(UPrimitiveComponent* OverlappedComponent
			, AActor* OtherActor
			, UPrimitiveComponent* OtherComp
			, int32 OtherBodyIndex
			, bool bFromSweep
			, const FHitResult & SweepResult);
};

