// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BowPowerWidget.h"

#include "Components/ProgressBar.h"

void UBowPowerWidget::SetPower(float Power, float MinPower, float MaxPower)
{
	PowerBar->SetPercent((Power - MinPower) / (MaxPower - MinPower));
}
