// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbstractClasses/Arrow/BasicArrowProjectile.h"

#include "Characters/MummyCharacter.h"
#include "Components/PointLightComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "MummyArcherGame/Public/Engine/Components/BasicProjectileMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#define PROJECTILE_OBJECT_TYPE ECC_GameTraceChannel1

ABasicArrowProjectile::ABasicArrowProjectile() 
{
	bReplicates = true;
	bAlwaysRelevant = true;
	
	InitialLifeSpan = 0.f;
	
	MaxSpeed = 10000.f;
	MinSpeed = 500.f;
	GravityScale = 1.f;
	
	Arrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow"));
	Arrow->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	Arrow->OnComponentHit.AddDynamic(this, &ABasicArrowProjectile::OnArrowHit);
	Arrow->SetCollisionProfileName(TEXT("Projectile"));
	Arrow->CanCharacterStepUpOn = ECB_No;
	SetRootComponent(Arrow);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLight->SetLightColor(FLinearColor(50.f, 255.f, 28.f));
	PointLight->SetAttenuationRadius(300.f);
	PointLight->SetupAttachment(RootComponent);

	ProjectileEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileEffect"));
	ProjectileEffect->SetupAttachment(RootComponent);
	
	BasicProjectileMovement = CreateDefaultSubobject<UBasicProjectileMovementComponent>(TEXT("ProjectileComponen"));
	BasicProjectileMovement->UpdatedComponent = Arrow;
	BasicProjectileMovement->bSimulationEnabled = false;
	BasicProjectileMovement->InitialSpeed = 0.f;
	BasicProjectileMovement->MaxSpeed = MaxSpeed;
	BasicProjectileMovement->bRotationFollowsVelocity = true;
	BasicProjectileMovement->bShouldBounce = false;
}

void ABasicArrowProjectile::Server_Fire(const FTransform& InTransform, float Speed)
{
	SetActorRotation(InTransform.GetRotation());
	SetActorLocation(InTransform.GetLocation());
	BasicProjectileMovement->Velocity = InTransform.GetRotation().Vector() * Speed;
	BasicProjectileMovement->bSimulationEnabled = true;
	Arrow->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Multicast_Fire(InTransform, BasicProjectileMovement->Velocity);
}

void ABasicArrowProjectile::Multicast_Fire_Implementation(const FTransform& InTransform, const FVector& InVelocity)
{
	if(GetOwner()->HasAuthority()) return;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	SetActorRotation(InTransform.GetRotation());
	SetActorLocation(InTransform.GetLocation());
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FlashEffect, GetActorLocation(), GetActorRotation());
	BasicProjectileMovement->Velocity = InVelocity;
	BasicProjectileMovement->bSimulationEnabled = true;
	Arrow->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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
	Arrow->SetVisibility(false, true);
}

void ABasicArrowProjectile::OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == nullptr || OtherActor == this) return;

	if(ABasicCharacter* OtherCharacter = Cast<ABasicCharacter>(OtherActor))
	{
		AttachToComponent(OtherCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, Hit.BoneName);
		UGameplayStatics::ApplyDamage(OtherCharacter, 10.f, nullptr, this, nullptr);
	}
	else
	{
		AttachToActor(OtherActor, FAttachmentTransformRules::KeepWorldTransform);
	}

	if(GetOwner()->HasAuthority())
	{
		Server_ArrowRelativeTransform = Arrow->GetRelativeTransform();
		Server_RootRelativeTransform = RootComponent->GetRelativeTransform();
		SetLifeSpan(5.f);
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

	auto* test = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, GetActorLocation(), GetActorRotation());
	//DrawDebugSphere(GetWorld(), GetActorLocation(), 20.f, 8, FColor::Green, false, 5.f);
	PointLight->SetIntensity(0.f);
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

void ABasicArrowProjectile::SetIgnoredActor(AActor* InActor) const
{
	Arrow->IgnoreActorWhenMoving(InActor, true);
}

void ABasicArrowProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABasicArrowProjectile, Server_ArrowRelativeTransform)
	DOREPLIFETIME(ABasicArrowProjectile, Server_RootRelativeTransform)
}
