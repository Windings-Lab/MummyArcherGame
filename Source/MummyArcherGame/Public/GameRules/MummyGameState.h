// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MummyPlayerState.h"
#include "Characters/MummyCharacter.h"
#include "GameFramework/GameState.h"
#include "Kismet/GameplayStatics.h"
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

	void OnPlayerKill() { 
		for (int i = 0; i < PlayerArray.Num(); i++)
		{
			int32 Team = Cast<AMummyPlayerState>(PlayerArray[i])->Team;
			if(!Cast<AMummyCharacter>(PlayerArray[i]->GetPawn())->IsDead()
				&& TeamsAlivePlayers.Num() > 0)
			{
				TeamsAlivePlayers[Team]++;
			}
			
		}
		for (int i = 0; i < TeamsAlivePlayers.Num(); i++)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("num %d "), TeamsAlivePlayers[i]));
			if(TeamsAlivePlayers[i] == 0)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Team %d killed"), i));
			}
		}
		
	};
	
protected:
	virtual void HandleBeginPlay() override;
	
	TArray<int32> TeamsAlivePlayers;
	
};
