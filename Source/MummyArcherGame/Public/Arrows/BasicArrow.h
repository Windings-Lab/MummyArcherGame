// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "BasicArrow.generated.h"

/**
 * 
 */
UCLASS()
class MUMMYARCHERGAME_API ABasicArrow : public ABasicArrowProjectile
{
	GENERATED_BODY()

protected:
	virtual void OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
};
