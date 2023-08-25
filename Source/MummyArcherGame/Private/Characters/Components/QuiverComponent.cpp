// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Components/QuiverComponent.h"

#include "Arrows/TeleportationArrow.h"

TSubclassOf<ABasicArrowProjectile> UQuiverComponent::GetArrow(TEnumAsByte<Arrow::EType> ArrowType)
{
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


