// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArrowProjectile.h"

#include "FirstPersonCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

void AArrowProjectile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	auto* PlayerOwner = Cast<AFirstPersonCharacter>(GetOwner());
	if(PlayerOwner)
	{
		Arrow->IgnoreActorWhenMoving(PlayerOwner, true);
	}
}

AArrowProjectile::AArrowProjectile() 
{
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComponent"));
	RootComponent = DefaultSceneRoot;
	
	Arrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow"));
	Arrow->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	Arrow->CanCharacterStepUpOn = ECB_No;
	Arrow->SetupAttachment(RootComponent);

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &AArrowProjectile::OnArrowBeginOverlap);
	BoxCollider->SetupAttachment(Arrow);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = Arrow;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// Die after 3 seconds by default
	InitialLifeSpan = 5.f;
}

void AArrowProjectile::OnArrowBeginOverlap(UPrimitiveComponent* OverlappedComponent
		, AActor* OtherActor
		, UPrimitiveComponent* OtherComp
		, int32 OtherBodyIndex
		, bool bFromSweep
		, const FHitResult & SweepResult)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->ProjectileGravityScale = 0.f;

		// Arrow will be stuck with collided actor
		const FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);
		AttachToActor(OtherActor, AttachmentTransformRules);
		BoxCollider->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}
}