// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Components/QuiverComponent.h"

#include "Arrows/TeleportationArrow.h"
#include "Arrows/BasicArrow.h"

UQuiverComponent::UQuiverComponent()
{
	int ArrowTypeCount = Arrow::EType::Max;
	ArrowCount.Init(0, ArrowTypeCount);
}

TSubclassOf<ABasicArrowProjectile> UQuiverComponent::GetArrow(Arrow::EType ArrowType)
{
	if(ArrowCount[ArrowType] <= 0) return nullptr;
	
	switch (ArrowType)
	{
	case Arrow::EType::Basic:
		return BasicArrow;
	case Arrow::Teleportation:
		return TeleportationArrow;
	default:
		check(false);
		return nullptr;
	}
}

int UQuiverComponent::GetArrowCount(Arrow::EType ArrowType)
{
	return ArrowCount[ArrowType];
}

void UQuiverComponent::SetArrowCount(Arrow::EType ArrowType, int Count)
{
	ArrowCount[ArrowType] = Count;
}

void UQuiverComponent::Decrease(Arrow::EType ArrowType)
{
	ArrowCount[ArrowType]--;
}
