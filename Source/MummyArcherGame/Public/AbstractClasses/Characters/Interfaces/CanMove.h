// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CanMove.generated.h"

UINTERFACE(MinimalAPI)
class UCanMove : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MUMMYARCHERGAME_API ICanMove
{
	GENERATED_BODY()
public:
	UFUNCTION()
	virtual void Move(const struct FInputActionValue& Value) = 0;

	UFUNCTION()
	virtual class UInputAction* GetMoveAction() = 0;
};
