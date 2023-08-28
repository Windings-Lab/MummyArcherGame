// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Interfaces/AffectedByWind.h"
#include "BasicArrowProjectile.generated.h"

UENUM(BlueprintType, Blueprintable)
namespace Arrow
{
	enum EType
	{
		Basic = 0,
		Teleportation,
		Max
	};
}

USTRUCT()
struct FProjectileBounds
{
	GENERATED_BODY()

	FProjectileBounds(const FVector& InStart, const FVector& InEnd) : Min(InStart), Max(InEnd) {}
	FProjectileBounds() : FProjectileBounds(FVector(), FVector()) {}

	FVector Min;
	FVector Max;

	FProjectileBounds& operator+=(const FVector& MoveDelta)
	{
		const FVector NewMin = Min + MoveDelta;
		Max += (NewMin - Min).GetSafeNormal() * MoveDelta.Length();
		Min = NewMin;

		return *this;
	}

	FProjectileBounds operator+(const FVector& MoveDelta) const
	{
		FProjectileBounds Result = *this;
		Result += MoveDelta;

		return Result;
	}
};

USTRUCT()
struct FProjectileParams
{
	GENERATED_BODY()
	
	FProjectileParams(const FTransform& InTransform, const FProjectileBounds& InBounds, const FVector& InImpactPoint, float InSpeed, float InMaxSpeed, FVector InAcceleration)
		: Transform(InTransform), Bounds(InBounds), ImpactPoint(InImpactPoint), Speed(InSpeed), MaxSpeed(InMaxSpeed), Acceleration(InAcceleration)
	{
		Transform.SetScale3D(FVector::One());
	}

	FProjectileParams() : FProjectileParams(FTransform(), FProjectileBounds(), FVector(), 0.f, 0.f, FVector())
	{}

	FTransform Transform;
	FProjectileBounds Bounds;
	FVector ImpactPoint;

	float Speed;
	float MaxSpeed;
	FVector Acceleration;
};

UCLASS(Abstract, config=Game)
class ABasicArrowProjectile : public AActor, public IAffectedByWind
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Components, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Arrow;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Components, meta = (AllowPrivateAccess = "true"))
	class UPointLightComponent* PointLight;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Components, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* ProjectileEffect;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UBasicProjectileMovementComponent* BasicProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=VFXSettings, meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* HitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=VFXSettings, meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* FlashEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=ArrowSettings, meta = (AllowPrivateAccess = "true"))
	float MinSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=ArrowSettings, meta = (AllowPrivateAccess = "true"))
	float MaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=ArrowSettings, meta = (AllowPrivateAccess = "true"))
	float GravityScale;

public:
	ABasicArrowProjectile();

	FORCEINLINE class UBasicProjectileMovementComponent* GetProjectileMovement() const { return BasicProjectileMovement; }
	FORCEINLINE class UStaticMesh* GetMesh() const { return Arrow->GetStaticMesh(); }
	FORCEINLINE float GetMinSpeed() const { return MinSpeed; }
	FORCEINLINE float GetMaxSpeed() const { return MaxSpeed; }
	FORCEINLINE float GetGravityScale() const { return GravityScale; }

	float GetWidth() const;
	FProjectileBounds GetBounds(const FVector& Location, const FVector& Direction) const;
	FProjectileBounds GetBounds() const;
	float CalculateArrowSpeed(float BowTensionTime, float BowMaxTensionTime) const;

	void Server_Fire(const FTransform& InTransform, float Speed);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FTransform& InTransform, const FVector& InVelocity);

	void SetIgnoredActor(AActor* InActor) const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// Called when client arrow still not collided with something, when server arrow is collided
	virtual void OnRep_AttachmentReplication() override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
protected:
	virtual void SetWindModificator(const FVector& Vector) override;
	UFUNCTION()
	virtual void OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(Replicated)
	FTransform Server_RootRelativeTransform;

	UPROPERTY(Replicated)
	FTransform Server_ArrowRelativeTransform;
};

