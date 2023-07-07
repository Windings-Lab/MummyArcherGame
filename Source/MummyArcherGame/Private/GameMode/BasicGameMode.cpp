// Copyright Epic Games, Inc. All Rights Reserved.

#include "..\..\Public\GameMode\BasicGameMode.h"
#include "UObject/ConstructorHelpers.h"

ABasicGameMode::ABasicGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_BasicCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
