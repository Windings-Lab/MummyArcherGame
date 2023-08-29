// Fill out your copyright notice in the Description page of Project Settings.


#include "Arrows/BasicArrow.h"

#include "AI/FlyingAI.h"
#include "Characters/MummyCharacter.h"
#include "Characters/Components/QuiverComponent.h"

void ABasicArrow::OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                             FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnArrowHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

	if(OtherActor->IsA<AFlyingAI>())
	{
		if(AMummyCharacter* Character = Cast<AMummyCharacter>(GetOwner()))
		{
			UQuiverComponent* Quiver = Character->GetQuiver();
			Quiver->SetArrowCount(Arrow::Basic, Quiver->GetArrowCount(Arrow::Basic) + 5);
			Quiver->SetArrowCount(Arrow::Teleportation, Quiver->GetArrowCount(Arrow::Teleportation) + 5);
			Destroy();
			OtherActor->Destroy();
		}
	}
}
