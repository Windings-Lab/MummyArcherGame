// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameRules/BasicGameMode.h"

#include "GameRules/MummyGameState.h"
#include "UObject/ConstructorHelpers.h"

ABasicGameMode::ABasicGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Characters/Blueprints/BP_MummyCharacter"));
	static ConstructorHelpers::FClassFinder<AMummyGameState> GameStateClassFinder(TEXT("/Game/GameRules/BP_MummyGameState"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	GameStateClass   = GameStateClassFinder.Class;
}
