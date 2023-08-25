// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "QuiverComponent.generated.h"

namespace Arrow
{
	enum EType : int;
}

/**
 * 
 */
UCLASS()
class MUMMYARCHERGAME_API UQuiverComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ABasicArrowProjectile> BasicArrow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATeleportationArrow> TeleportationArrow;

public:
	TSubclassOf<ABasicArrowProjectile> GetArrow(TEnumAsByte<Arrow::EType> ArrowType);
};
