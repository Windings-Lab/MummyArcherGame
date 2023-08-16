// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BasicGameMode.generated.h"

class AMummyPlayerController;
class AMummyPlayerStart;

UCLASS(minimalapi)
class ABasicGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABasicGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	//virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual bool ShouldSpawnAtStartSpot(AController* Player) override { return false; }

	//UPROPERTY(EditAnywhere, Category = "Defaults")
	//int32 TeamCount = 2;


	//UPROPERTY(EditAnywhere, Category = "Defaults")
	//int32 PlayersNeedToConnect = 4;

	int32 TempInt = 1;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	int32 CurrentTeam = 0;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	int32 NumberOfPlayersInTeam = 3;

	TArray<AMummyPlayerController*> PlayerControllers;
};



