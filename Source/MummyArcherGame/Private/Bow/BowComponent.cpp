// Copyright Epic Games, Inc. All Rights Reserved.


#include "Bow/BowComponent.h"

#include "Bow/Widgets/BowPowerWidget.h"
#include "Bow/ArrowProjectile.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Mummy/MummyCharacter.h"

// Sets default values for this component's properties
UBowComponent::UBowComponent()
{
	TimeUntilMaxPower = 3.f;

	ArrowMaxSpeed = 10000.f;
	ArrowMinSpeed = 500.f;
	ArrowGravityScale = 1.f;
}

void UBowComponent::Focus(const FInputActionValue& Value)
{
	bool Focused = Value.Get<bool>();
	auto* Camera = Character->GetFollowCamera();
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
	
	PowerScale = CalculateArrowSpeed(ArrowMinSpeed, ArrowMaxSpeed, ActionInstance.GetElapsedTime());
	//PowerScale = 4000.f;
	
	FPredictProjectilePathResult ProjectilePathResult;
	PredictArrowPath(World, ProjectilePathResult);
	
	if(BowPowerWidget) BowPowerWidget->SetPower(PowerScale, ArrowMinSpeed, ArrowMaxSpeed);
}

void UBowComponent::FireButtonPresses(const FInputActionInstance& ActionInstance)
{
	if(BowPowerWidget) BowPowerWidget->AddToViewport();
}

void UBowComponent::Fire(const FInputActionInstance& ActionInstance)
{
	if (!Character
		|| !Character->GetController()
		|| !ProjectileClass) return;
	
	UWorld* const World = GetWorld(); 
	if (!World) return;
	
	CreateArrow(World);
}

void UBowComponent::FireButtonReleased(const FInputActionInstance& ActionInstance)
{
	if(BowPowerWidget) BowPowerWidget->RemoveFromParent();
}

FTransform UBowComponent::CalculateArrowTransform()
{
	FTransform Transform = GetSocketTransform(TEXT("arrow_socket"));
	Transform.SetScale3D(FVector::One());
	Transform.SetRotation(UKismetMathLibrary::MakeRotFromX(ImpactPoint - Transform.GetLocation()).Quaternion());

	return Transform;
}

float UBowComponent::CalculateArrowSpeed(float MinPower, float MaxPower, const float HoldTime) const
{
	float Power;
	const float PowerDifference = MaxPower - MinPower;
	if(HoldTime <= TimeUntilMaxPower)
	{
		Power =  PowerDifference * HoldTime / TimeUntilMaxPower + MinPower;
	}
	else
	{
		Power = ArrowMaxSpeed;
	}
	
	return Power;
}

void UBowComponent::PredictArrowPath(UWorld* const World, FPredictProjectilePathResult& ProjectilePathResult)
{
	FHitResult TraceLineHitResult;
	BowTraceLine(World, 10000.0, false, TraceLineHitResult);
	
	FTransform SpawnTransform	= CalculateArrowTransform();
	FVector SpawnLocation		= SpawnTransform.GetLocation();
	
	FVector Velocity = FVector::Zero();
	FCollisionResponseParams CollisionResponseParams;
	CollisionResponseParams.CollisionResponse.SetResponse(ECC_Visibility, ECR_Block);
		
	UGameplayStatics::SuggestProjectileVelocity(World, Velocity, SpawnLocation, ImpactPoint, ArrowMaxSpeed
	, false, 2
	, -980 * ArrowGravityScale
	, ESuggestProjVelocityTraceOption::Type::DoNotTrace
	, CollisionResponseParams
	, {Character}
	, false);
	
	FPredictProjectilePathParams ProjectilePathParams = FPredictProjectilePathParams(
		2.f
		, SpawnLocation
		, Velocity.GetSafeNormal() * PowerScale,
		10.f
		, ECC_Visibility, GetOwner());
	ProjectilePathParams.DrawDebugType = EDrawDebugTrace::Type::ForDuration;
	ProjectilePathParams.DrawDebugTime = 0.f;
	ProjectilePathParams.SimFrequency = 30.f;
	ProjectilePathParams.bTraceComplex = false;
	ProjectilePathParams.OverrideGravityZ = -980 * ArrowGravityScale;

	UGameplayStatics::PredictProjectilePath(GetWorld(), ProjectilePathParams, ProjectilePathResult);
	InitialArrowDirection = ProjectilePathResult.PathData[0].Velocity;
}

// DrawDebugSphere(World, SocketLocation, 9.f, 8, FColor::Yellow, false, 5);

void UBowComponent::CreateArrow(UWorld* const World)
{
	FTransform SpawnTransform = CalculateArrowTransform();
	SpawnTransform.SetRotation(InitialArrowDirection.ToOrientationQuat());

	auto* Arrow = World->SpawnActorDeferred<AArrowProjectile>(ProjectileClass
															  , SpawnTransform
															  , Character
															  , nullptr
															  , ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			
	auto* ArrowMovement = Arrow->GetProjectileMovement();
	ArrowMovement->MaxSpeed = ArrowMaxSpeed;
	ArrowMovement->InitialSpeed = InitialArrowDirection.Length();
	ArrowMovement->ProjectileGravityScale = ArrowGravityScale;
	UGameplayStatics::FinishSpawningActor(Arrow, SpawnTransform);
}

void UBowComponent::BowTraceLine(UWorld* const World, double Distance, bool DrawTrace, FHitResult& HitResult)
{
	const auto* Camera = Character->GetFollowCamera();
	const FVector CameraForwardVector = Camera->GetForwardVector();
	
	const FVector CameraLocation = Camera->GetComponentLocation();

	FVector TraceStartLocation = CameraLocation;
	const FVector TraceEndLocation = TraceStartLocation + CameraForwardVector * Distance;
	
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(Character);

	if(DrawTrace)
	{
		FName TraceTag = FName(TEXT("CharacterTraceTag"));
		CollisionQueryParams.TraceTag = TraceTag;
		World->DebugDrawTraceTag = TraceTag;
	}
	
	World->LineTraceSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, ECC_Visibility, CollisionQueryParams);

	ImpactPoint = HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceEndLocation;
}

void UBowComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if(SightWidgetClass)
	{
		SightWidget = CreateWidget<UUserWidget>(GetWorld(), SightWidgetClass);
	}

	if(BowPowerWidgetClass)
	{
		BowPowerWidget = Cast<UBowPowerWidget>(CreateWidget<UUserWidget>(GetWorld(), BowPowerWidgetClass));
	}
}

void UBowComponent::AttachBowToCharacter(AMummyCharacter* TargetCharacter)
{
	if (!TargetCharacter) return;

	Character = TargetCharacter;

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
			EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Started, this, &UBowComponent::FireButtonPresses);
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
	if (!Character) return;

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}
