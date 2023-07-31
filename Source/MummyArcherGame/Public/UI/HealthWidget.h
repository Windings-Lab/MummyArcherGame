// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthWidget.generated.h"

class UProgressBar;

UCLASS()
class MUMMYARCHERGAME_API UHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateHealth(float Percent);

protected:
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UProgressBar> HealthBar;
	
};
