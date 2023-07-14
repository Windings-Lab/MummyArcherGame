// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BowPowerWidget.generated.h"

/**
 * 
 */
UCLASS()
class MUMMYARCHERGAME_API UBowPowerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPower(float Power, float MinPower, float MaxPower);
	
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UProgressBar* PowerBar;
	
};
