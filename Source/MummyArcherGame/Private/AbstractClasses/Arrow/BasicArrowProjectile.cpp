// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbstractClasses/Arrow/BasicArrowProjectile.h"

#include "Characters/MummyCharacter.h"
#include "MummyArcherGame/Public/Engine/Components/BasicProjectileMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ABasicArrowProjectile::ABasicArrowProjectile() 
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 5.f;
	
	MaxSpeed = 10000.f;
	MinSpeed = 500.f;
	GravityScale = 1.f;
	
	Arrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow"));
	Arrow->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	Arrow->OnComponentBeginOverlap.AddDynamic(this, &ABasicArrowProjectile::OnArrowBeginOverlap);
	Arrow->OnComponentHit.AddDynamic(this, &ABasicArrowProjectile::OnArrowHit);
	Arrow->CanCharacterStepUpOn = ECB_No;
	SetRootComponent(Arrow);
	
	BasicProjectileMovement = CreateDefaultSubobject<UBasicProjectileMovementComponent>(TEXT("ProjectileComponen"));
	BasicProjectileMovement->UpdatedComponent = Arrow;
	BasicProjectileMovement->InitialSpeed = 0.f;
	BasicProjectileMovement->MaxSpeed = MaxSpeed;
	BasicProjectileMovement->bRotationFollowsVelocity = true;
	BasicProjectileMovement->bShouldBounce = false;
}

void ABasicArrowProjectile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	BasicProjectileMovement->ProjectileGravityScale = GravityScale;
	BasicProjectileMovement->MaxSpeed = MaxSpeed;
}

void ABasicArrowProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void ABasicArrowProjectile::OnArrowBeginOverlap(UPrimitiveComponent* OverlappedComponent
                                                , AActor* OtherActor
                                                , UPrimitiveComponent* OtherComp
                                                , int32 OtherBodyIndex
                                                , bool bFromSweep
                                                , const FHitResult & SweepResult)
{
	
}

void ABasicArrowProjectile::OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == nullptr || OtherActor == this) return;

	OnHitWithActor(OtherActor);
}

void ABasicArrowProjectile::OnHitWithActor(AActor* OtherActor)
{
	AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform);

	if(GetOwner()->HasAuthority())
	{
		Server_ArrowRelativeTransform = Arrow->GetRelativeTransform();
		Server_RootRelativeTransform = RootComponent->GetRelativeTransform();
	}

	if(AMummyCharacter* OtherCharacter = Cast<AMummyCharacter>(OtherActor))
	{
		UGameplayStatics::ApplyDamage(OtherCharacter, 10.f, nullptr, this, nullptr);
	}
}

void ABasicArrowProjectile::SetWindModificator(const FVector& Vector)
{
	BasicProjectileMovement->SetWindModificator(Vector);
}

void ABasicArrowProjectile::OnRep_AttachmentReplication()
{
	Super::OnRep_AttachmentReplication();

	RootComponent->SetRelativeTransform(Server_RootRelativeTransform);
	Arrow->SetRelativeTransform(Server_ArrowRelativeTransform);
}

void ABasicArrowProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

float ABasicArrowProjectile::GetWidth() const
{
	return FVector::Dist(Arrow->GetComponentLocation(), Arrow->GetSocketLocation("ArrowHead"));
}

FProjectileBounds ABasicArrowProjectile::GetBounds(const FVector& Location, const FVector& Direction) const
{
	FVector Min = Location;
	FVector Max = Min + Direction * GetWidth();

	return FProjectileBounds(Min, Max);
}

FProjectileBounds ABasicArrowProjectile::GetBounds() const
{
	return GetBounds(Arrow->GetComponentLocation(), Arrow->GetForwardVector());
}

float ABasicArrowProjectile::CalculateArrowSpeed(const float BowTensionTime, const float BowMaxTensionTime) const
{
	if(BowMaxTensionTime == 0.f) return GetMaxSpeed();
	
	float ClampedTime = FMath::Clamp(BowTensionTime, 0.f, BowMaxTensionTime);
	float DeltaSpeed = GetMaxSpeed() - GetMinSpeed();
	float Speed = (DeltaSpeed * ClampedTime / BowMaxTensionTime) + GetMinSpeed();
	
	return Speed;
}

void ABasicArrowProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABasicArrowProjectile, Server_ArrowRelativeTransform)
	DOREPLIFETIME(ABasicArrowProjectile, Server_RootRelativeTransform)
}
