// Fill out your copyright notice in the Description page of Project Settings.


#include "GameRules/MummyGameState.h"

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
}
