// Copyright Epic Games, Inc. All Rights Reserved.


#include "Bow/Components/BowComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Bow/Widgets/BowPowerWidget.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "AbstractClasses/Characters/BasicCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Engine/ProjectilePathPredictor.h"
#include "Engine/Components/BasicProjectileMovementComponent.h"
#include "GameRules/MummyGameState.h"
#include "Kismet/GameplayStatics.h"

UBowComponent::UBowComponent()
{
	bWantsInitializeComponent = true;
	MaxBowTensionTime = .5f;

	ArrowPathPredictor = CreateDefaultSubobject<UProjectilePathPredictor>(TEXT("ArrowPathPrediction"));
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
		SightWidget->RemoveFromParent();
	}
}

void UBowComponent::FireButtonPressed()
{
	if(BowPowerWidget) BowPowerWidget->AddToViewport();
}

FProjectileParams UBowComponent::CreateArrowParams(float BowTensionTime)
{
	FHitResult TraceLineHitResult;

	AMummyGameState* GameState = GetWorld()->GetGameState<AMummyGameState>();
	FVector WindAcceleration = FVector::Zero();
	if(GameState)
	{
		WindAcceleration = GameState->GetWindAcceleration();
	}
	FVector Acceleration = FVector(0.f, 0.f, -980 * ArrowCDO->GetGravityScale()) + WindAcceleration;
	
	FProjectileParams ArrowParameters(GetSocketTransform(TEXT("arrow_socket"))
		, ArrowCDO->GetBounds()
		, Pawn->TraceLine(false, TraceLineHitResult)
		, ArrowCDO->CalculateArrowSpeed(BowTensionTime, MaxBowTensionTime)
		, ArrowCDO->GetMaxSpeed()
		, Acceleration);
	
	FVector Direction = ArrowPathPredictor->GetInitialArrowDirection(*GetWorld(), ArrowParameters, {GetOwner()});
	ArrowParameters.Transform.SetRotation(Direction.ToOrientationQuat());
	ArrowParameters.Bounds = ArrowCDO->GetBounds(GetSocketLocation(TEXT("arrow_socket")), Direction);

	return ArrowParameters;
}

void UBowComponent::FireButtonHolding(const FInputActionInstance& ActionInstance)
{
	FProjectileParams ArrowParams = CreateArrowParams(ActionInstance.GetElapsedTime());

	FPredictProjectilePathResult ProjectilePathResult;
	ArrowPathPredictor->PredictProjectilePathWithWind(*GetWorld(), ArrowParams, {GetOwner()}, ProjectilePathResult);
	
	if(BowPowerWidget) BowPowerWidget->SetPower(ArrowParams.Speed, ArrowCDO->GetMinSpeed(), ArrowCDO->GetMaxSpeed());
}

void UBowComponent::Fire(const FInputActionInstance& ActionInstance)
{
	FProjectileParams ArrowParams = CreateArrowParams(ActionInstance.GetElapsedTime());

	FPredictProjectilePathResult ProjectilePathResult;
	ArrowPathPredictor->PredictProjectilePathWithWind(*GetWorld(), ArrowParams, {GetOwner()}, ProjectilePathResult);
	
	ArrowCDO->GetProjectileMovement()->InitialSpeed = (ArrowParams.Transform.GetRotation().Vector() * ArrowParams.Speed).Length();
	Fire(ArrowParams.Transform);
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
	if(BowPowerWidget) BowPowerWidget->RemoveFromParent();
}
