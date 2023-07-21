// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameRules/BasicGameMode.h"
#include "UObject/ConstructorHelpers.h"

ABasicGameMode::ABasicGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Characters/Blueprints/BP_MummyCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
