// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbstractClasses/Arrow/BasicArrowProjectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABasicArrowProjectile::ABasicArrowProjectile() 
{
	InitialLifeSpan = 5.f;
	
	MaxSpeed = 10000.f;
	MinSpeed = 500.f;
	GravityScale = 1.f;
	
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneComponent"));
	RootComponent = RootSceneComponent;
	
	Arrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow"));
	Arrow->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	Arrow->CanCharacterStepUpOn = ECB_No;
	Arrow->SetupAttachment(RootComponent);

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ABasicArrowProjectile::OnArrowBeginOverlap);
	BoxCollider->SetupAttachment(Arrow);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = Arrow;
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
}

void ABasicArrowProjectile::OnArrowBeginOverlap(UPrimitiveComponent* OverlappedComponent
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

void ABasicArrowProjectile::PredictArrowPath(UWorld* const World
                                           , FVector& VelocityDirection
                                           , const FVector& SpawnLocation
                                           , const FVector& EndLocation
                                           , const float Speed
                                           , FPredictProjectilePathResult& ProjectilePathResult) const
{
	FVector Velocity = FVector::Zero();
	FCollisionResponseParams CollisionResponseParams;
	CollisionResponseParams.CollisionResponse.SetResponse(ECC_Visibility, ECR_Block);
		
	UGameplayStatics::SuggestProjectileVelocity(World, Velocity, SpawnLocation, EndLocation, GetMaxSpeed()
	, false, 2
	, -980 * GetGravityScale()
	, ESuggestProjVelocityTraceOption::Type::DoNotTrace
	, CollisionResponseParams
	, {GetOwner()}
	, false);
	
	FPredictProjectilePathParams ProjectilePathParams = FPredictProjectilePathParams(
		2.f
		, SpawnLocation
		, Velocity.GetSafeNormal() * Speed,
		10.f
		, ECC_Visibility, GetOwner());
	ProjectilePathParams.DrawDebugType = EDrawDebugTrace::Type::ForDuration;
	ProjectilePathParams.DrawDebugTime = 0.f;
	ProjectilePathParams.SimFrequency = 30.f;
	ProjectilePathParams.bTraceComplex = false;
	ProjectilePathParams.OverrideGravityZ = -980 * GetGravityScale();

	UGameplayStatics::PredictProjectilePath(World, ProjectilePathParams, ProjectilePathResult);

	if(!ProjectilePathResult.PathData.IsEmpty())
		VelocityDirection = ProjectilePathResult.PathData[0].Velocity;
}

float ABasicArrowProjectile::CalculateArrowSpeed(const float BowTensionTime, const float BowMaxTensionTime) const
{
	float ClampedTime = FMath::Clamp(BowTensionTime, 0.f, BowMaxTensionTime);
	float PowerDifference = GetMaxSpeed() - GetMinSpeed();
	float Power = (PowerDifference * ClampedTime / BowMaxTensionTime) + GetMinSpeed();
	
	return Power;
}
