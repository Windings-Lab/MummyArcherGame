// Copyright Epic Games, Inc. All Rights Reserved.

#include "MummyArcherGame/Public/GameMode/MummyArcherGameGameMode.h"
#include "UObject/ConstructorHelpers.h"

AMummyArcherGameGameMode::AMummyArcherGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
