// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Controllers/MummyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"

//void AMummyPlayerController::Respawn()
//{
//	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
//	if (GameMode)
//	{
//		APawn* NewPawn = GameMode->SpawnDefaultPawnFor(this, GameMode->ChoosePlayerStart(this));
//	}
//}
//
//void AMummyPlayerController::SetPositionInTeamOnServer_Implementation(int32 PositionInTeam)
//{
//	this->PositionInTeam = PositionInTeam;
//}
//
//void AMummyPlayerController::SetTeamNumberOnServer_Implementation(int32 TeamNumber)
//{
//	this->TeamNumber = TeamNumber;
//}
//
//void AMummyPlayerController::OnKilled()
//{
//	UnPossess();
//	InRate = 5.0f;
//	GetWorldTimerManager().SetTimer(TimerHandleRespawn, this, &AMummyPlayerController::Respawn, InRate, false);
//}

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
	FString DebugMessage = FString::Printf(TEXT("%d, %s"), PositionInTeam, *PlayerState->GetPlayerName());
	AddWidgetOnClient(DebugMessage);

}

void AMummyPlayerController::AddWidgetOnClient_Implementation(const FString& Message)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, Message);
}
