// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MummyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MUMMYARCHERGAME_API AMummyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int32 Team;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;
	
};
