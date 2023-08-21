// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "MummyPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class MUMMYARCHERGAME_API AMummyPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	int32 Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	bool bIsFree = true;
	
};
