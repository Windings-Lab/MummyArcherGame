// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbstractClasses/Arrow/BasicArrowProjectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#define TRACE_CHARACTER ECC_GameTraceChannel2

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
	BoxCollider->SetUseCCD(true);
	BoxCollider->SetupAttachment(Arrow);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = Arrow;
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
}

void ABasicArrowProjectile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	ProjectileMovement->ProjectileGravityScale = GravityScale;
	ProjectileMovement->MaxSpeed = MaxSpeed;
}

void ABasicArrowProjectile::OnArrowBeginOverlap(UPrimitiveComponent* OverlappedComponent
                                                , AActor* OtherActor
                                                , UPrimitiveComponent* OtherComp
                                                , int32 OtherBodyIndex
                                                , bool bFromSweep
                                                , const FHitResult & SweepResult)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->ProjectileGravityScale = 0.f;
		
		const FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::KeepWorld, true);
		AttachToActor(OtherActor, AttachmentTransformRules);
		BoxCollider->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}
}

void ABasicArrowProjectile::PredictArrowPath(UWorld* const World
											, FArrowParameters& ArrowParameters
											, bool DrawArc
											, FPredictProjectilePathResult& ProjectilePathResult) const
{
	if(GetGravityScale() == 0.f) return;
	
	FVector Velocity = ArrowParameters.ImpactPoint - ArrowParameters.SpawnTransform.GetLocation();
		
	FCollisionResponseParams CollisionResponseParams;
	CollisionResponseParams.CollisionResponse.SetResponse(TRACE_CHARACTER, ECR_Block);
		
	UGameplayStatics::SuggestProjectileVelocity(World, Velocity, ArrowParameters.SpawnTransform.GetLocation(), ArrowParameters.ImpactPoint, GetMaxSpeed()
	                                            , false, 2
	                                            , -980 * GetGravityScale()
	                                            , ESuggestProjVelocityTraceOption::Type::DoNotTrace
	                                            , CollisionResponseParams
	                                            , {GetOwner()}
	                                            , DrawArc);

	FPredictProjectilePathParams ProjectilePathParams = FPredictProjectilePathParams(
		2.f
		, ArrowParameters.SpawnTransform.GetLocation()
		, Velocity.GetSafeNormal() * ArrowParameters.Speed,
		10.f
		, TRACE_CHARACTER, GetOwner());
	ProjectilePathParams.DrawDebugType = EDrawDebugTrace::Type::ForDuration;
	ProjectilePathParams.DrawDebugTime = 0.f;
	ProjectilePathParams.SimFrequency = 30.f;
	ProjectilePathParams.bTraceComplex = false;
	ProjectilePathParams.OverrideGravityZ = -980 * GetGravityScale();

	UGameplayStatics::PredictProjectilePath(World, ProjectilePathParams, ProjectilePathResult);
}

float ABasicArrowProjectile::CalculateArrowSpeed(const float BowTensionTime, const float BowMaxTensionTime) const
{
	float ClampedTime = FMath::Clamp(BowTensionTime, 0.f, BowMaxTensionTime);
	float PowerDifference = GetMaxSpeed() - GetMinSpeed();
	float Power = (PowerDifference * ClampedTime / BowMaxTensionTime) + GetMinSpeed();
	
	return Power;
}
