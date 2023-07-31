// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/GameHUDWidget.h"
#include "MummyHUD.generated.h"



UCLASS()
class MUMMYARCHERGAME_API AMummyHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = Widgets)
		FORCEINLINE	UGameHUDWidget* GetMainWidget() const { return GameHUDWidget; };

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Widgets)
		TSubclassOf<UGameHUDWidget> GameHUDWidgetClass;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite, Category = Widgets)
		TObjectPtr<UGameHUDWidget> GameHUDWidget;
};
