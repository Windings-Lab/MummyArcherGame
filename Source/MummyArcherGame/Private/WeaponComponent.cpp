// Copyright Epic Games, Inc. All Rights Reserved.


#include "WeaponComponent.h"

#include "BowPowerWidget.h"
#include "FirstPersonCharacter.h"
#include "ArrowProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}


void UWeaponComponent::FireButtonPresses(const FInputActionInstance& ActionInstance)
{
	if(BowPowerWidget) BowPowerWidget->AddToViewport();
}

void UWeaponComponent::Fire(const FInputActionInstance& ActionInstance)
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	// Try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetSocketLocation("Muzzle") + SpawnRotation.RotateVector(MuzzleOffset);
			FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);
	
			// Spawn the projectile at the muzzle
			auto* Arrow = World->SpawnActorDeferred<AArrowProjectile>(ProjectileClass
				, SpawnTransform
				, Character
				, nullptr
				, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			auto* ArrowMovement = Arrow->GetProjectileMovement();
			ArrowMovement->MaxSpeed = ArrowMaxSpeed;
			ArrowMovement->InitialSpeed = CalculateArrowSpeed(ArrowMinSpeed, ArrowMaxSpeed, ActionInstance.GetElapsedTime());
			UGameplayStatics::FinishSpawningActor(Arrow, SpawnTransform);}

			if(BowPowerWidget) BowPowerWidget->SetPower(ForceScale, ArrowMinSpeed, ArrowMaxSpeed);
	}
}

void UWeaponComponent::FireButtonReleased(const FInputActionInstance& ActionInstance)
{
	if(BowPowerWidget) BowPowerWidget->RemoveFromParent();
}

void UWeaponComponent::CalculateArrowPath(const FInputActionInstance& ActionInstance)
{
	ForceScale = CalculateArrowSpeed(ArrowMinSpeed, ArrowMaxSpeed, ActionInstance.GetElapsedTime());

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
	const FVector SpawnLocation = GetSocketLocation("Muzzle") + SpawnRotation.RotateVector(MuzzleOffset);

	FPredictProjectilePathParams ProjectilePathParams = FPredictProjectilePathParams(
		10.f
		, SpawnLocation
		, SpawnRotation.Vector() * ForceScale,
		10.f
		, ECC_WorldStatic, GetOwner());
	ProjectilePathParams.DrawDebugType = EDrawDebugTrace::Type::ForDuration;
	ProjectilePathParams.DrawDebugTime = 0.f;
	ProjectilePathParams.SimFrequency = 30.f;
	ProjectilePathParams.bTraceComplex = false;
	ProjectilePathParams.OverrideGravityZ = -980;

	FPredictProjectilePathResult ProjectilePathResult;

	UGameplayStatics::PredictProjectilePath(GetWorld(), ProjectilePathParams, ProjectilePathResult);

	if(BowPowerWidget) BowPowerWidget->SetPower(ForceScale, ArrowMinSpeed, ArrowMaxSpeed);
}

float UWeaponComponent::CalculateArrowSpeed(float MinPower, float MaxPower, float ElapsedTime) const
{
	float Power;
	const float PowerDifference = MaxPower - MinPower;
	if(ElapsedTime <= FireActionStruct.TimeUntilMaxPower)
	{
		Power =  PowerDifference * ElapsedTime / FireActionStruct.TimeUntilMaxPower + MinPower;
	}
	else if(ElapsedTime <= FireActionStruct.TimeToForceShoot)
	{
		float TToShoot = FireActionStruct.TimeToForceShoot - FireActionStruct.TimeUntilMaxPower;
		float T = TToShoot - (ElapsedTime - FireActionStruct.TimeUntilMaxPower);
		Power = PowerDifference * T / TToShoot + MinPower;
	}
	else
	{
		Power = MinPower;
	}
	
	return Power;
}

void UWeaponComponent::Focus(const FInputActionValue& Value)
{
	if(!SightWidget) return;
	
	bool Focused = Value.Get<bool>();
	if(Focused)
	{
		SightWidget->AddToViewport();
	}
	else
	{
		SightWidget->RemoveFromParent();
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

void UWeaponComponent::AttachWeapon(AFirstPersonCharacter* TargetCharacter)
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
			EnhancedInputComponent->BindAction(FireActionStruct.FireAction, ETriggerEvent::Ongoing, this, &UWeaponComponent::CalculateArrowPath);
			
			EnhancedInputComponent->BindAction(FocusActionStruct.FocusAction, ETriggerEvent::Triggered, this, &UWeaponComponent::Focus);
		}
	}
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
