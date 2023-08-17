// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameRules/BasicGameMode.h"


#include "EngineUtils.h"
#include "AbstractClasses/Characters/BasicCharacter.h"
#include "Characters/MummyCharacter.h"
#include "Characters/Controllers/MummyPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameRules/MummyPlayerStart.h"
#include "GameRules/MummyPlayerState.h"
#include "UObject/ConstructorHelpers.h"
#include "GameRules/MummyGameState.h"



ABasicGameMode::ABasicGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Characters/Blueprints/BP_MummyCharacter"));
	static ConstructorHelpers::FClassFinder<AMummyGameState> GameStateClassFinder(TEXT("/Game/GameRules/BP_MummyGameState"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	GameStateClass   = GameStateClassFinder.Class;
	PlayerStateClass = AMummyPlayerState::StaticClass();
}

void ABasicGameMode::PostLogin(APlayerController* NewPlayer)
{

	Super::PostLogin(NewPlayer);
	AMummyPlayerController* Player = Cast<AMummyPlayerController>(NewPlayer);
	Player->SetTeamNumberOnServer(CurrentTeam);
	Player->SetPositionInTeamOnServer(TempInt);
	if (TempInt == NumberOfPlayersInTeam)
	{
		TempInt = 1;
		CurrentTeam++;
	}
	else
	{
		TempInt++;
	}
	PlayerControllers.Add(Player);
	Player->AddWidgetOnServer();
	
}
