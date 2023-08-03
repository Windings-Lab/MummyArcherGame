// Fill out your copyright notice in the Description page of Project Settings.


#include "GameRules/MummyPlayerState.h"

#include "Net/UnrealNetwork.h"

void AMummyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMummyPlayerState, Team);
}
