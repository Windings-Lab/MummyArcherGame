// Copyright Epic Games, Inc. All Rights Reserved.


#include "Characters/Components/BowComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UI/BowPowerWidget.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "AbstractClasses/Characters/BasicCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GameHUDWidget.h"
#include "UI/MummyHUD.h"

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

	GameHUDWidget = Cast<AMummyHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())->GetMainWidget();
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
		//if(!SightWidget) return;
		//SightWidget->AddToViewport();
	}
	else
	{
		Camera->SetFieldOfView(90.f);
		//if(!SightWidget) return;
		//SightWidget->AddToViewport();
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
	
	if(GameHUDWidget) GameHUDWidget->GetBowPowerWidget()->SetPower(ArrowParameters.Speed, ArrowCDO->GetMinSpeed(), ArrowCDO->GetMaxSpeed());
}

void UBowComponent::FireButtonPressed()
{
	if(GameHUDWidget) GameHUDWidget->ShowBowPower();
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
	ArrowCDO->GetProjectileMovement()->InitialSpeed = InitialSpeed;

	Fire(ArrowParameters.SpawnTransform);
}

void UBowComponent::Fire(const FTransform& SpawnTransform)
{
	if(Pawn->HasAuthority())
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Owner = Pawn;
		ActorSpawnParameters.Instigator = Pawn;
		ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		GetWorld()->SpawnActor(ArrowProjectileClass, &SpawnTransform, ActorSpawnParameters);
	}
	else
	{
		Server_Fire(SpawnTransform);
	}
}

void UBowComponent::Server_Fire_Implementation(const FTransform& SpawnTransform)
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = Pawn;
	ActorSpawnParameters.Instigator = Pawn;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	GetWorld()->SpawnActor(ArrowProjectileClass, &SpawnTransform, ActorSpawnParameters);
}

void UBowComponent::FireButtonReleased()
{
	if(GameHUDWidget) GameHUDWidget->HideBowPower();
}
