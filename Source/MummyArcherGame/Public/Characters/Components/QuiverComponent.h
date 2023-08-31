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
	TSubclassOf<class ABasicArrow> BasicArrow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATeleportationArrow> TeleportationArrow;

	TArray<int> ArrowCount;

public:
	UQuiverComponent();
	
	TSubclassOf<class ABasicArrowProjectile> GetArrow(Arrow::EType ArrowType);

	UFUNCTION(BlueprintCallable)
	int GetArrowCount(Arrow::EType ArrowType);

	void SetArrowCount(Arrow::EType ArrowType, int Count);
	
	void Decrease(Arrow::EType ArrowType);
};
