// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HealthWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHealthWidget::UpdateHealth(float Percent, int32 Points)
{
	if (HealthBar && HealthPoints)
	{
		HealthBar->SetPercent(Percent);

		HealthPoints->SetText( FText::FromString(FString::FromInt(Points)) );
	}
}

void UHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();
}
