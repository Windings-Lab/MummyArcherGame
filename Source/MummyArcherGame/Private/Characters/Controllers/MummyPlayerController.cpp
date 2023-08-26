// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Controllers/MummyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Characters/MummyCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"


void AMummyPlayerController::SetTeamNumberOnServer_Implementation(int32 TeamNumberVar)
{
	TeamNumber = TeamNumberVar;
	
}

void AMummyPlayerController::SetPositionInTeamOnServer_Implementation(int32 PositionInTeamVar)
{
	PositionInTeam = PositionInTeamVar;
}

void AMummyPlayerController::AddWidgetOnServer_Implementation()
{
	FString DebugMessage = FString::Printf(TEXT("%d, %s"), TeamNumber, *PlayerState->GetPlayerName());
	AddWidgetOnClient(DebugMessage, TeamNumber, PositionInTeam);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, DebugMessage);
	AMummyCharacter* TempMummyCharacter = Cast<AMummyCharacter>(GetCharacter());
	if (TempMummyCharacter)
	{
		TempMummyCharacter->TeamNumber = TeamNumber;
		TempMummyCharacter->PositionInTeam = PositionInTeam;
	}
}

void AMummyPlayerController::AddWidgetOnClient_Implementation(const FString& Message, int32 TeamNumberLocal, int32 PositionInTeamLocal)
{
	PositionInTeam = PositionInTeamLocal;
	TeamNumber = TeamNumberLocal;
	AMummyCharacter* TempMummyCharacter = Cast<AMummyCharacter>(GetCharacter());
	if (TempMummyCharacter)
	{
		TempMummyCharacter->TeamNumber = TeamNumber;
		TempMummyCharacter->PositionInTeam = PositionInTeam;

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, Message);
	}
}

