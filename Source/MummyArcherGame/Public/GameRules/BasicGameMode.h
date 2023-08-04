// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BasicGameMode.generated.h"

class AMummyPlayerStart;

UCLASS(minimalapi)
class ABasicGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABasicGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual bool ShouldSpawnAtStartSpot(AController* Player) override { return false; }

	UPROPERTY(EditAnywhere, Category = "Defaults")
	int32 TeamCount = 2;
};



