// Copyright Epic Games, Inc. All Rights Reserved.


#include "WeaponComponent.h"

#include "BowPowerWidget.h"
#include "ArrowProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	FireActionStruct.TimeUntilMaxPower = 3.f;
	FireActionStruct.TimeToForceShoot  = 6.f;

	ArrowMaxSpeed = 10000.f;
	ArrowMinSpeed = 500.f;
}


void UWeaponComponent::FireButtonPresses(const FInputActionInstance& ActionInstance)
{
	if(BowPowerWidget) BowPowerWidget->AddToViewport();
}

void UWeaponComponent::Fire(const FInputActionInstance& ActionInstance)
{
	if (!Character
		|| !Character->GetController()
		|| !ProjectileClass) return;
	
	UWorld* const World = GetWorld(); 
	if (!World) return;

	BowTraceLine(World);
	CreateArrow(World, ActionInstance.GetElapsedTime());
}

void UWeaponComponent::BowTraceLine(UWorld* const World)
{
	auto* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
	FVector TraceStartLocation = PlayerCameraManager->K2_GetActorLocation();

	FVector CameraForwardVector = PlayerCameraManager->GetActorForwardVector();
	FVector TraceEndLocation = TraceStartLocation + CameraForwardVector * FVector(15000, 15000, 15000);
	ImpactPoint = TraceEndLocation;
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(Character);
	World->LineTraceSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, ECC_Visibility, CollisionQueryParams);

	if(HitResult.bBlockingHit)
	{
		ImpactPoint = HitResult.ImpactPoint;
	}
}

void UWeaponComponent::FireButtonReleased(const FInputActionInstance& ActionInstance)
{
	if(BowPowerWidget) BowPowerWidget->RemoveFromParent();
}

void UWeaponComponent::FireButtonHolding(const FInputActionInstance& ActionInstance)
{
	BowTraceLine(GetWorld());
	PowerScale = CalculateArrowSpeed(ArrowMinSpeed, ArrowMaxSpeed, ActionInstance.GetElapsedTime());
	FTransform SpawnTransform	= CalculateArrowTransform();
	FVector SpawnLocation		= SpawnTransform.GetLocation();
	FRotator SpawnRotation		= SpawnTransform.Rotator();

	FPredictProjectilePathParams ProjectilePathParams = FPredictProjectilePathParams(
		10.f
		, SpawnLocation
		, SpawnRotation.Vector() * PowerScale,
		10.f
		, ECC_WorldStatic, GetOwner());
	ProjectilePathParams.DrawDebugType = EDrawDebugTrace::Type::ForDuration;
	ProjectilePathParams.DrawDebugTime = 0.f;
	ProjectilePathParams.SimFrequency = 30.f;
	ProjectilePathParams.bTraceComplex = false;
	ProjectilePathParams.OverrideGravityZ = -980;

	FPredictProjectilePathResult ProjectilePathResult;

	UGameplayStatics::PredictProjectilePath(GetWorld(), ProjectilePathParams, ProjectilePathResult);

	if(BowPowerWidget) BowPowerWidget->SetPower(PowerScale, ArrowMinSpeed, ArrowMaxSpeed);
}

float UWeaponComponent::CalculateArrowSpeed(float MinPower, float MaxPower, float HoldTime) const
{
	float Power;
	const float PowerDifference = MaxPower - MinPower;
	if(HoldTime <= FireActionStruct.TimeUntilMaxPower)
	{
		Power =  PowerDifference * HoldTime / FireActionStruct.TimeUntilMaxPower + MinPower;
	}
	else if(HoldTime <= FireActionStruct.TimeToForceShoot)
	{
		float TToShoot = FireActionStruct.TimeToForceShoot - FireActionStruct.TimeUntilMaxPower;
		float T = TToShoot - (HoldTime - FireActionStruct.TimeUntilMaxPower);
		Power = PowerDifference * T / TToShoot + MinPower;
	}
	else
	{
		Power = MinPower;
	}
	
	return Power;
}

FTransform UWeaponComponent::CalculateArrowTransform() const
{
	FTransform Transform = GetSocketTransform(TEXT("arrow_socket"));
	Transform.SetScale3D(UE::Math::TVector<double>(1.f, 1.f, 1.f));
	Transform.SetRotation(UKismetMathLibrary::MakeRotFromX(ImpactPoint - Transform.GetLocation()).Quaternion());

	return Transform;
}

void UWeaponComponent::CreateArrow(UWorld* const World, const float HoldTime)
{
	const FTransform SpawnTransform = CalculateArrowTransform();

	auto* Arrow = World->SpawnActorDeferred<AArrowProjectile>(ProjectileClass
															  , SpawnTransform
															  , Character
															  , nullptr
															  , ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			
	auto* ArrowMovement = Arrow->GetProjectileMovement();
	ArrowMovement->MaxSpeed = ArrowMaxSpeed;
	ArrowMovement->InitialSpeed = CalculateArrowSpeed(ArrowMinSpeed, ArrowMaxSpeed, HoldTime);
	UGameplayStatics::FinishSpawningActor(Arrow, SpawnTransform);
}

void UWeaponComponent::Focus(const FInputActionValue& Value)
{
	if(!SightWidget) return;
	auto* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	
	bool Focused = Value.Get<bool>();
	if(Focused)
	{
		SightWidget->AddToViewport();
		PlayerCameraManager->SetFOV(60.f);
	}
	else
	{
		SightWidget->RemoveFromParent();
		PlayerCameraManager->SetFOV(90.f);
	}
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if(FocusActionStruct.SightWidgetClass)
	{
		SightWidget = CreateWidget<UUserWidget>(GetWorld(), FocusActionStruct.SightWidgetClass);
	}

	if(BowPowerWidgetClass)
	{
		BowPowerWidget = Cast<UBowPowerWidget>(CreateWidget<UUserWidget>(GetWorld(), BowPowerWidgetClass));
	}
}

void UWeaponComponent::AttachWeapon(ACharacter* TargetCharacter)
{
	Character = TargetCharacter;
	if (Character == nullptr)
	{
		return;
	}

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(FireActionStruct.FireAction, ETriggerEvent::Started, this, &UWeaponComponent::FireButtonPresses);
			EnhancedInputComponent->BindAction(FireActionStruct.FireAction, ETriggerEvent::Triggered, this, &UWeaponComponent::Fire);
			EnhancedInputComponent->BindAction(FireActionStruct.FireAction, ETriggerEvent::Completed, this, &UWeaponComponent::FireButtonReleased);
			EnhancedInputComponent->BindAction(FireActionStruct.FireAction, ETriggerEvent::Canceled, this, &UWeaponComponent::FireButtonReleased);
			EnhancedInputComponent->BindAction(FireActionStruct.FireAction, ETriggerEvent::Ongoing, this, &UWeaponComponent::FireButtonHolding);
			
			EnhancedInputComponent->BindAction(FocusActionStruct.FocusAction, ETriggerEvent::Triggered, this, &UWeaponComponent::Focus);

			int test =5;
		}
	}
}

void UWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}
