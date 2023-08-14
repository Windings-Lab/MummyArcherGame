// Fill out your copyright notice in the Description page of Project Settings.


#include "GameRules/MummyGameState.h"

#include "GameRules/BasicGameMode.h"

AMummyGameState::AMummyGameState()
{
	RandomizeWindDirection();
	WindDirection.Z = 0.f;

	WindSpeed = 1000.f;
}

void AMummyGameState::RandomizeWindDirection()
{
	WindDirection = FMath::VRand();
}

void AMummyGameState::HandleBeginPlay()
{
	bReplicatedHasBegunPlay = true;

	GetWorldSettings()->NotifyBeginPlay();
	GetWorldSettings()->NotifyMatchStarted();

	for (int i = 0; i < Cast<ABasicGameMode>(GetDefaultGameMode())->TeamCount; i++)
	{
		TeamsAlivePlayers.Add(0);
	}
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Team %d"), TeamsAlivePlayers.Num()));
}
