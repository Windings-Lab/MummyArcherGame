// Copyright Epic Games, Inc. All Rights Reserved.


#include "Bow/Components/BowComponent.h"

#include "Bow/Widgets/BowPowerWidget.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/MummyCharacter.h"

void UBowComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if(SightWidgetClass)
	{
		SightWidget = CreateWidget<UUserWidget>(GetWorld(), SightWidgetClass);
	}

	if(BowPowerWidgetClass)
	{
		BowPowerWidget = Cast<UBowPowerWidget>(CreateWidget<UUserWidget>(GetWorld(), BowPowerWidgetClass));
	}

	if(ArrowProjectileClass)
	{
		ArrowCDO = Cast<ABasicArrowProjectile>(ArrowProjectileClass->ClassDefaultObject);
	}
}

UBowComponent::UBowComponent()
{
	bWantsInitializeComponent = true;
	MaxBowTensionTime = .5f;
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
	UWorld* const World = GetWorld(); 
	if (!World) return;

	const auto* Camera = Pawn->GetFollowCamera();
	const FVector CameraForwardVector = Camera->GetForwardVector();
	const FVector CameraLocation = Camera->GetComponentLocation();

	const FVector TraceStartLocation = CameraLocation;
	const FVector TraceEndLocation = TraceStartLocation + CameraForwardVector * 10000.f;

	FHitResult TraceLineHitResult;
	TraceLine(World, TraceStartLocation, TraceEndLocation, false, TraceLineHitResult);
	
	FPredictProjectilePathResult ProjectilePathResult;
	ArrowCDO->PredictArrowPath(World
		, InitialArrowDirection
		, GetSocketLocation("arrow_socket")
		, TraceLineHitResult.bBlockingHit ? TraceLineHitResult.ImpactPoint: TraceEndLocation
		, ArrowCDO->CalculateArrowSpeed(ActionInstance.GetElapsedTime(), MaxBowTensionTime)
		, ProjectilePathResult);
	
	if(BowPowerWidget) BowPowerWidget->SetPower(BowPowerScale, ArrowCDO->GetMinSpeed(), ArrowCDO->GetMaxSpeed());
}

void UBowComponent::FireButtonPressed()
{
	if(BowPowerWidget) BowPowerWidget->AddToViewport();
}

void UBowComponent::Fire()
{
	UWorld* const World = GetWorld(); 
	if (!World) return;
	
	CreateArrow(World);
}

void UBowComponent::FireButtonReleased()
{
	if(BowPowerWidget) BowPowerWidget->RemoveFromParent();
}

FTransform UBowComponent::CalculateArrowTransform()
{
	FTransform Transform = GetSocketTransform(TEXT("arrow_socket"));
	Transform.SetScale3D(FVector::One());
	Transform.SetRotation(InitialArrowDirection.ToOrientationQuat());

	return Transform;
}

// DrawDebugSphere(World, SocketLocation, 9.f, 8, FColor::Yellow, false, 5);

void UBowComponent::CreateArrow(UWorld* const World)
{
	const FTransform SpawnTransform = CalculateArrowTransform();

	auto* Arrow = World->SpawnActorDeferred<ABasicArrowProjectile>(ArrowProjectileClass
		, SpawnTransform
		, Pawn
		, nullptr
		, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	
	auto* ArrowMovement = Arrow->GetProjectileMovement();
	ArrowMovement->InitialSpeed = InitialArrowDirection.Length();
	UGameplayStatics::FinishSpawningActor(Arrow, SpawnTransform);
}

void UBowComponent::TraceLine(UWorld* const World, const FVector& Start, const FVector& End, const bool DrawTrace, FHitResult& HitResult)
{
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(Pawn);

	if(DrawTrace)
	{
		FName TraceTag = FName(TEXT("CharacterTraceTag"));
		CollisionQueryParams.TraceTag = TraceTag;
		World->DebugDrawTraceTag = TraceTag;
	}
	
	World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionQueryParams);
}

void UBowComponent::AttachBowToCharacter(AMummyCharacter* TargetCharacter)
{
	if(!TargetCharacter) return;
	
	Pawn = TargetCharacter;

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(BowMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Started, this, &UBowComponent::FireButtonPressed);
			EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Triggered, this, &UBowComponent::Fire);
			EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Completed, this, &UBowComponent::FireButtonReleased);
			EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Canceled, this, &UBowComponent::FireButtonReleased);
			EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Ongoing, this, &UBowComponent::FireButtonHolding);
			
			EnhancedInputComponent->BindAction(BowFocusAction, ETriggerEvent::Triggered, this, &UBowComponent::Focus);
		}
	}
}

void UBowComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(BowMappingContext);
		}
	}
}
