// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Gameplay/Interfaces/AffectedByWind.h"

#include "BasicProjectileMovementComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), ShowCategories=(Velocity))
class MUMMYARCHERGAME_API UBasicProjectileMovementComponent : public UProjectileMovementComponent, public IAffectedByWind
{
	GENERATED_UCLASS_BODY()

public:
	virtual void SetWindModificator(const FVector& Vector) override;

protected:
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual FVector ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const override;
	virtual FVector ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const override;
	virtual FVector ComputeVelocity(FVector InitialVelocity, float DeltaTime) const override;

	FORCEINLINE FVector GetVelocityFromMoveDelta(const FVector& MoveDelta, const FVector& V0, float DT);

private:
	FVector WindModificator;
	FVector OverridedMoveDelta;
	FVector OverridedNewVelocity;
};
