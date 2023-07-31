// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HealthWidget.h"

#include "Components/ProgressBar.h"

void UHealthWidget::UpdateHealth(float Percent)
{
	HealthBar->SetPercent(Percent);
}
