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
	if(NewPlayer)
	{
		AMummyPlayerState* PS = Cast<AMummyPlayerState>(NewPlayer->PlayerState);
		if(PS && GameState)
		{
			TArray<int> Teams;
			Teams.Init(0, TeamCount);
			
			for (APlayerState* State : GameState->PlayerArray)
			{
				AMummyPlayerState* OtherPS = Cast<AMummyPlayerState>(State);
				if (OtherPS)
				{
					Teams[OtherPS->Team]++;
				}
			}

			PS->Team = 0;
			for (int32 i = 0; i < Teams.Num(); i++)
			{
				if (Teams[i] < Teams[PS->Team])
				{
					PS->Team = i;
				}
			}
		}
	}
	
	Super::PostLogin(NewPlayer);
}

AActor* ABasicGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	Super::ChoosePlayerStart_Implementation(Player);
	
	if (Player)
	{
		AMummyPlayerState* PS = Cast<AMummyPlayerState>(Player->PlayerState);
		if (PS)
		{
			TArray<AMummyPlayerStart*> Starts;
			for (TActorIterator<AMummyPlayerStart> It(GetWorld()); It; ++It)
			{
				AMummyPlayerStart* Start = *It;
				if (Start && Start->Team == PS->Team && Start->bIsFree)
				{
					Starts.Add(Start);
				}
			}
			
			AMummyPlayerStart* Start = Starts[FMath::RandRange(0, Starts.Num() - 1)];
			if (!Player->HasAuthority())
			{
				Start->bIsFree = false;
			}
			return Start;
		}		
	}
	return nullptr;
}
