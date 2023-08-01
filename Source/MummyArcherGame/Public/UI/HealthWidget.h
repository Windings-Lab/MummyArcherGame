// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class MUMMYARCHERGAME_API UHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateHealth(float Percent, int32 Points);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> HealthPoints;
};
