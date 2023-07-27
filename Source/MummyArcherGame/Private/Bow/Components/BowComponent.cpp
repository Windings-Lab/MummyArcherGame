// Copyright Epic Games, Inc. All Rights Reserved.


#include "Bow/Components/BowComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Bow/Widgets/BowPowerWidget.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "AbstractClasses/Characters/BasicCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UBowComponent::UBowComponent()
{
	bWantsInitializeComponent = true;
	MaxBowTensionTime = .5f;
}

void UBowComponent::InitializeComponent()
{
	Super::InitializeComponent();

	ABasicCharacter* BasicPawn = Cast<ABasicCharacter>(GetOwner());
	if(!BasicPawn) return;

	Pawn = BasicPawn;

	if(ArrowProjectileClass)
	{
		ArrowCDO = Cast<ABasicArrowProjectile>(ArrowProjectileClass->ClassDefaultObject);
	}
}

void UBowComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Started, this, &UBowComponent::FireButtonPressed);
		EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Triggered, this, &UBowComponent::Fire);
		EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Completed, this, &UBowComponent::FireButtonReleased);
		EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Canceled, this, &UBowComponent::FireButtonReleased);
		EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Ongoing, this, &UBowComponent::FireButtonHolding);
			
		EnhancedInputComponent->BindAction(BowFocusAction, ETriggerEvent::Triggered, this, &UBowComponent::Focus);
	}

	if(SightWidgetClass)
	{
		SightWidget = CreateWidget<UUserWidget>(GetWorld(), SightWidgetClass);
	}

	if(BowPowerWidgetClass)
	{
		BowPowerWidget = Cast<UBowPowerWidget>(CreateWidget<UUserWidget>(GetWorld(), BowPowerWidgetClass));
	}
}

void UBowComponent::AddBowMappingContext(UEnhancedInputLocalPlayerSubsystem* Subsystem, int Priority)
{
	Subsystem->AddMappingContext(BowMappingContext, Priority);
}

void UBowComponent::RemoveBowMappingContext(UEnhancedInputLocalPlayerSubsystem* Subsystem)
{
	Subsystem->RemoveMappingContext(BowMappingContext);
}

void UBowComponent::Focus(const FInputActionValue& Value)
{
	const bool Focused = Value.Get<bool>();
	auto* Camera = Pawn->GetFollowCamera();
	if(Focused)
	{

		Camera->SetFieldOfView(30.f);
		if(!SightWidget) return;
		SightWidget->AddToViewport();
	}
	else
	{
		Camera->SetFieldOfView(90.f);
		if(!SightWidget) return;
		SightWidget->AddToViewport();
	}
}

void UBowComponent::FireButtonHolding(const FInputActionInstance& ActionInstance)
{
	FHitResult TraceLineHitResult;
	FVector TraceImpactPoint = Pawn->TraceLine(false, TraceLineHitResult);
	
	FPredictProjectilePathResult ProjectilePathResult;
	FArrowParameters ArrowParameters(ArrowProjectileClass, GetSocketTransform("arrow_socket"), TraceImpactPoint
		, ArrowCDO->CalculateArrowSpeed(ActionInstance.GetElapsedTime(), MaxBowTensionTime));
	
	ArrowCDO->PredictArrowPath(GetWorld(),ArrowParameters, false, ProjectilePathResult);
	
	if(BowPowerWidget) BowPowerWidget->SetPower(ArrowParameters.Speed, ArrowCDO->GetMinSpeed(), ArrowCDO->GetMaxSpeed());
}

void UBowComponent::FireButtonPressed()
{
	if(BowPowerWidget) BowPowerWidget->AddToViewport();
}

void UBowComponent::Fire(const FInputActionInstance& ActionInstance)
{
	FHitResult TraceLineHitResult;

	FArrowParameters ArrowParameters(ArrowProjectileClass, GetSocketTransform("arrow_socket")
		, Pawn->TraceLine(false, TraceLineHitResult)
		, ArrowCDO->CalculateArrowSpeed(ActionInstance.GetElapsedTime(), MaxBowTensionTime));

	FPredictProjectilePathResult ProjectilePathResult;
	ArrowCDO->PredictArrowPath(GetWorld(), ArrowParameters, false, ProjectilePathResult);

	FVector InitialVelocityDirection = ArrowParameters.ImpactPoint - ArrowParameters.SpawnTransform.GetLocation();
	float InitialSpeed = 0.f;
	if(ArrowCDO->GetGravityScale() != 0.f && !ProjectilePathResult.PathData.IsEmpty())
	{
		InitialVelocityDirection = ProjectilePathResult.PathData[0].Velocity;
		InitialSpeed = InitialVelocityDirection.Length();
	}
	else
	{
		InitialVelocityDirection = InitialVelocityDirection.GetSafeNormal();
		InitialSpeed = ArrowParameters.Speed;
	}
	
	ArrowParameters.SpawnTransform.SetRotation(InitialVelocityDirection.ToOrientationQuat());
	
	if(Pawn->HasAuthority())
	{
		Multicast_Fire(ArrowParameters.SpawnTransform, InitialSpeed);
	}
	else
	{
		Server_Fire(ArrowParameters.SpawnTransform, InitialSpeed);
	}
}

void UBowComponent::Server_Fire_Implementation(const FTransform& SpawnTransform, float InitialSpeed)
{
	Multicast_Fire(SpawnTransform, InitialSpeed);
}

void UBowComponent::Multicast_Fire_Implementation(const FTransform& SpawnTransform, float InitialSpeed)
{
	auto* Arrow = GetWorld()->SpawnActorDeferred<ABasicArrowProjectile>(ArrowProjectileClass
		, SpawnTransform
		, Pawn
		, Pawn
		, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	Arrow->GetProjectileMovement()->InitialSpeed = InitialSpeed;
	UGameplayStatics::FinishSpawningActor(Arrow, SpawnTransform);
}

void UBowComponent::FireButtonReleased()
{
	if(BowPowerWidget) BowPowerWidget->RemoveFromParent();
}
