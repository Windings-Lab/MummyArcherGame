// Copyright Epic Games, Inc. All Rights Reserved.

#include "..\..\Public\GameMode\BasicGameMode.h"
#include "UObject/ConstructorHelpers.h"

ABasicGameMode::ABasicGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
