// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CanJump.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCanJump : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MUMMYARCHERGAME_API ICanJump
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual class UInputAction* GetJumpAction() = 0;
};
