// Fill out your copyright notice in the Description page of Project Settings.


#include "Arrows/TeleportationArrow.h"

#include "AbstractClasses/Characters/BasicCharacter.h"

void ATeleportationArrow::OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == nullptr || OtherActor == this) return;
	
	if(ABasicCharacter* Character = Cast<ABasicCharacter>(GetOwner()))
	{
		Character->TeleportTo(Hit.Location, Character->GetActorRotation());
	}

	SetLifeSpan(.01f);
}
