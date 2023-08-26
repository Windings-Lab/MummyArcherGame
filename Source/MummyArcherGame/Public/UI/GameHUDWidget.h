// Fill out your copyright notice in the Description page of Project Settings.GetBowPowerWidget

#pragma once

#include "CoreMinimal.h"
#include "HealthWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameHUDWidget.generated.h"

class UHealthWidget;
//class UBowPowerWidget;

UCLASS()
class MUMMYARCHERGAME_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		FORCEINLINE	UHealthWidget* GetHealthWidget() const { return HealthWidget; };


protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UHealthWidget> HealthWidget;
	
};
