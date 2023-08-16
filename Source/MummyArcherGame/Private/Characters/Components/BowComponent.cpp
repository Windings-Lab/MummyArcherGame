// Copyright Epic Games, Inc. All Rights Reserved.


#include "Characters/Components/BowComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UI/BowPowerWidget.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "AbstractClasses/Characters/BasicCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/ProjectilePathPredictor.h"
#include "Engine/Components/BasicProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/GameHUDWidget.h"
#include "UI/MummyHUD.h"

UBowComponent::UBowComponent()
{
	bWantsInitializeComponent = true;
	MaxBowTensionTime = .5f;

	ArrowPathPredictor = CreateDefaultSubobject<UProjectilePathPredictor>(TEXT("ArrowPathPrediction"));
	
	ArrowSplinePath = CreateDefaultSubobject<USplineComponent>(TEXT("ArrowSplinePath"));
	ArcEndSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArcEndSphere"));
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

void UBowComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UBowComponent, bFocused, COND_SkipOwner)
	DOREPLIFETIME_CONDITION(UBowComponent, bFirePressed, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UBowComponent, bBowTensionIdle, COND_SkipOwner);
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
	bFocused = Value.Get<bool>();
	auto* Camera = Pawn->GetFollowCamera();
	if(bFocused)
	{
		Camera->SetFieldOfView(30.f);
		bBowTensionIdle = true;
	}
	else
	{
		Camera->SetFieldOfView(90.f);
		if(!bFirePressed)
		{
			bBowTensionIdle = false;
			bIsGetArrow = false;
			TimerBeforeGetArrow = 0.f;
		}
	}

	Server_Focus(bFocused, bBowTensionIdle);
}

void UBowComponent::Server_Focus_Implementation(bool InFocused, bool InBowTensionIdle)
{
	bFocused = InFocused;
	bBowTensionIdle = InBowTensionIdle;
}

void UBowComponent::FireButtonPressed()
{
	if(GameHUDWidget) GameHUDWidget->ShowBowPower();
	
	bBowTensionIdle = true;
	bFirePressed	= true;
	
	Server_FireButtonPressed();
}

void UBowComponent::Server_FireButtonPressed_Implementation()
{
	bBowTensionIdle = true;
	bFirePressed	= true;
}

void UBowComponent::FireButtonReleased()
{
	if(GameHUDWidget) GameHUDWidget->HideBowPower();
	
	ResetSpline();
	ArcEndSphere->SetVisibility(false, false);

	bFirePressed = false;
	bIsGetArrow = false;
	TimerBeforeGetArrow = 0.f;
	if(!bFocused)
	{
		bBowTensionIdle = false;
	}
	
	Server_FireButtonReleased(bBowTensionIdle);
}

void UBowComponent::Server_FireButtonReleased_Implementation(bool InBowTensionIdle)
{
	bFirePressed = false;
	bBowTensionIdle = InBowTensionIdle;
}

FProjectileParams UBowComponent::CreateArrowParams(float BowTensionTime)
{
	const FVector Acceleration = FVector(0.f, 0.f, -980 * ArrowCDO->GetGravityScale());
	
	FProjectileParams ArrowParameters(GetSocketTransform(TEXT("arrow_socket"))
		, ArrowCDO->GetBounds()
		, Pawn->GetAimLocation()
		, ArrowCDO->CalculateArrowSpeed(BowTensionTime, MaxBowTensionTime)
		, ArrowCDO->GetMaxSpeed()
		, Acceleration);

	const FVector Direction = ArrowPathPredictor->GetInitialArrowDirection(*GetWorld(), ArrowParameters, {GetOwner()});
	ArrowParameters.Transform.SetRotation(Direction.ToOrientationQuat());
	ArrowParameters.Bounds = ArrowCDO->GetBounds(GetSocketLocation(TEXT("arrow_socket")), Direction);

	return ArrowParameters;
}

void UBowComponent::FireButtonHolding(const FInputActionInstance& ActionInstance)
{
	if(!bIsGetArrow)
	{
		TimerBeforeGetArrow = ActionInstance.GetElapsedTime();
		return;
	}
	
	float TimerDelta = ActionInstance.GetElapsedTime() - TimerBeforeGetArrow;
	FProjectileParams ArrowParams = CreateArrowParams(TimerDelta);

	FPredictProjectilePathResult ProjectilePathResult;
	ArrowPathPredictor->PredictProjectilePathWithWind(*GetWorld(), ArrowParams, {GetOwner()}, ProjectilePathResult);

	ResetSpline();
	DrawSpline(ProjectilePathResult);
	
	if(GameHUDWidget) GameHUDWidget->GetBowPowerWidget()->SetPower(ArrowParams.Speed, ArrowCDO->GetMinSpeed(), ArrowCDO->GetMaxSpeed());
}

void UBowComponent::ResetSpline()
{
	ArrowSplinePath->ClearSplinePoints();
	for (auto* SplineMesh : SplineMeshes)
	{
		SplineMesh->DestroyComponent();
	}
	SplineMeshes.Empty();
}

void UBowComponent::DrawSpline(const FPredictProjectilePathResult& ProjectilePathResult)
{
	if(!ProjectilePathResult.PathData.IsEmpty() && ArcSplineMesh)
	{
		int LastIndex = ProjectilePathResult.PathData.Num() - 1;
		for (int i = 0; i <= LastIndex; i++)
		{
			ArrowSplinePath->AddSplinePoint(ProjectilePathResult.PathData[i].Location, ESplineCoordinateSpace::World, true);
			if(i == LastIndex)
			{
				ArrowSplinePath->SetSplinePointType(LastIndex, ESplinePointType::CurveClamped, true);
			}

			if(i == 0) continue;
			
			FVector StartLocation;
			FVector StartTangent;
			ArrowSplinePath->GetLocationAndTangentAtSplinePoint(i - 1, StartLocation, StartTangent, ESplineCoordinateSpace::World);

			FVector EndLocation;
			FVector EndTangent;
			ArrowSplinePath->GetLocationAndTangentAtSplinePoint(i, EndLocation, EndTangent, ESplineCoordinateSpace::World);
			
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			SplineMesh->RegisterComponent();
			SplineMesh->Mobility = EComponentMobility::Movable;
			SplineMesh->SetStaticMesh(ArcSplineMesh);
			SplineMesh->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent, true);
			SplineMeshes.Add(SplineMesh);
		}
		
		ArcEndSphere->SetWorldLocation(ProjectilePathResult.PathData.Last().Location);
		if(!ArcEndSphere->IsVisible())
		{
			ArcEndSphere->SetVisibility(true, false);
		}
	}
}

void UBowComponent::OnGetArrow()
{
	if(bFirePressed || bFocused)
	{
		bIsGetArrow = true;
	}
}

void UBowComponent::Fire(const FInputActionInstance& ActionInstance)
{
	if(!bIsGetArrow) return;

	float TimerDelta = ActionInstance.GetElapsedTime() - TimerBeforeGetArrow;
	FProjectileParams ArrowParams = CreateArrowParams(TimerDelta);

	FPredictProjectilePathResult ProjectilePathResult;
	ArrowPathPredictor->PredictProjectilePathWithWind(*GetWorld(), ArrowParams, {GetOwner()}, ProjectilePathResult);
	
	ArrowCDO->GetProjectileMovement()->InitialSpeed = (ArrowParams.Transform.GetRotation().Vector() * ArrowParams.Speed).Length();
	ArrowCDO->SetIgnoredActor(Pawn);
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
