// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Controllers/MummyPlayerController.h"

#include "GameFramework/GameModeBase.h"

void AMummyPlayerController::Respawn()
{
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (GameMode)
	{
		APawn* NewPawn = GameMode->SpawnDefaultPawnFor(this, GameMode->ChoosePlayerStart(this));
	}
}

void AMummyPlayerController::OnKilled()
{
	UnPossess();
	InRate = 5.0f;
	GetWorldTimerManager().SetTimer(TimerHandleRespawn, this, &AMummyPlayerController::Respawn, InRate, false);
}
