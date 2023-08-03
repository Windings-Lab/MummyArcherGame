// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MummyGameState.generated.h"

/**
 * 
 */
UCLASS()
class MUMMYARCHERGAME_API AMummyGameState : public AGameState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Wind", meta=(AllowPrivateAccess))
		FVector WindDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Wind", meta=(AllowPrivateAccess))
		float WindSpeed;

public:
	AMummyGameState();
	void RandomizeWindDirection();
	
	FORCEINLINE FVector GetWindAcceleration() const { return WindSpeed * WindDirection; }
	FORCEINLINE void SetWindDirection(const FVector& InDirection) { WindDirection = InDirection; }
	FORCEINLINE void SetWindSpeed(const float InSpeed) { WindSpeed = InSpeed; }
	
protected:
	virtual void HandleBeginPlay() override;
};
