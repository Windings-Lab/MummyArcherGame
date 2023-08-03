// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MummyHUD.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "UI/GameHUDWidget.h"

void AMummyHUD::BeginPlay()
{
	Super::BeginPlay();

	UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());

	check(GameHUDWidgetClass);

	GameHUDWidget = CreateWidget<UGameHUDWidget>(GetWorld(), GameHUDWidgetClass);

	if (IsValid(GameHUDWidget))
	{
		GameHUDWidget->AddToViewport();
	}
}
