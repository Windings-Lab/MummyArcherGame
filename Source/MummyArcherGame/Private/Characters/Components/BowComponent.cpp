// Copyright Epic Games, Inc. All Rights Reserved.


#include "Characters/Components/BowComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "Camera/CameraComponent.h"
#include "Characters/MummyCharacter.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/ProjectilePathPredictor.h"
#include "Engine/Components/BasicProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
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

	AMummyCharacter* BasicPawn = Cast<AMummyCharacter>(GetOwner());
	if(!BasicPawn) return;

	Pawn = BasicPawn;
}

void UBowComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Pawn->IsLocallyControlled())
	{
		ChangeArrow(Arrow::Basic);
	}
}

void UBowComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UBowComponent, bFocused, COND_SkipOwner)
	DOREPLIFETIME_CONDITION(UBowComponent, bBowTensionIdle, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UBowComponent, bFocusIdle, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UBowComponent, TensionPercent, COND_SkipOwner);
	DOREPLIFETIME(UBowComponent, bChangeArrow);

	DOREPLIFETIME_CONDITION(UBowComponent, CurrentArrow, COND_SkipOwner);
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
			
		EnhancedInputComponent->BindAction(BowFocusAction, ETriggerEvent::Triggered, this, &UBowComponent::FocusAction);
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

void UBowComponent::FocusAction(const FInputActionValue& Value)
{
	if(!CurrentArrow) return;
	
	if(Value.Get<bool>())
	{
		Focus();
	}
	else
	{
		Unfocus();
	}

	Server_Focus(bFocused, bFocusIdle);
}

void UBowComponent::Focus()
{
	bFocused = true;
	
	auto* Camera = Pawn->GetFollowCamera();
	Camera->SetFieldOfView(30.f);
	bFocusIdle = true;
}

void UBowComponent::Unfocus()
{
	bFocused = false;
	
	auto* Camera = Pawn->GetFollowCamera();
	Camera->SetFieldOfView(90.f);
	if(!bBowTensionIdle)
	{
		bFocusIdle = false;
		TimerBeforeGetArrow = 0.f;
	}
	Pawn->GetArrowFromQuiverMesh()->SetVisibility(false);
}

void UBowComponent::Server_Focus_Implementation(bool InFocused, bool InBowTensionIdle)
{
	bFocused = InFocused;
	bFocusIdle = InBowTensionIdle;

	if(!bFocused)
	{
		Pawn->GetArrowFromQuiverMesh()->SetVisibility(false);
	}
}

void UBowComponent::FireButtonPressed()
{
	if(!CurrentArrow) return;
	//if(GameHUDWidget) GameHUDWidget->ShowBowPower();
	bFocusIdle = true;
	bBowTensionIdle	= true;
	
	Server_FireButtonPressed();
}

void UBowComponent::Server_FireButtonPressed_Implementation()
{
	bFocusIdle = true;
	bBowTensionIdle	= true;
}

void UBowComponent::FireButtonReleased()
{
	if(!CurrentArrow) return;
	//if(GameHUDWidget) GameHUDWidget->HideBowPower();
	
	ResetSpline();
	ArcEndSphere->SetVisibility(false, false);
	
	Pawn->GetArrowOnBowTension()->SetVisibility(false);

	bBowTensionIdle = false;
	bChangeArrow = false;
	TimerBeforeGetArrow = 0.f;
	TensionPercent = 0.f;
	if(!bFocused)
	{
		bFocusIdle = false;
	}
	
	Server_FireButtonReleased(bFocusIdle);
}

void UBowComponent::Server_FireButtonReleased_Implementation(bool InBowTensionIdle)
{
	bBowTensionIdle = false;
	bChangeArrow = false;
	bFocusIdle = InBowTensionIdle;
	TensionPercent = 0.f;
	Pawn->GetArrowOnBowTension()->SetVisibility(false);
}

void UBowComponent::OnInterrupted()
{
	Unfocus();
	ResetSpline();
	ArcEndSphere->SetVisibility(false, false);
	
	Pawn->GetArrowOnBowTension()->SetVisibility(false);
	Pawn->GetArrowFromQuiverMesh()->SetVisibility(false);

	bBowTensionIdle = false;
	bChangeArrow = false;
	TimerBeforeGetArrow = 0.f;
	TensionPercent = 0.f;
	bFocusIdle = false;
	
	Server_OnInterrupted();
}

void UBowComponent::Server_OnInterrupted_Implementation()
{
	Pawn->GetArrowOnBowTension()->SetVisibility(false);
	Pawn->GetArrowFromQuiverMesh()->SetVisibility(false);

	bBowTensionIdle = false;
	bChangeArrow = false;
	TensionPercent = 0.f;
	bFocusIdle = false;
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
	if(!CurrentArrow) return;
	
	if(!bBowTensionIdle)
	{
		TimerBeforeGetArrow = ActionInstance.GetElapsedTime();
		return;
	}

	Pawn->GetArrowFromQuiverMesh()->SetVisibility(false);
	Pawn->GetArrowOnBowTension()->SetVisibility(true);
	
	float TimerDelta = ActionInstance.GetElapsedTime() - TimerBeforeGetArrow;
	FProjectileParams ArrowParams = CreateArrowParams(TimerDelta);

	FPredictProjectilePathResult ProjectilePathResult;
	ArrowPathPredictor->PredictProjectilePathWithWind(*GetWorld(), ArrowParams, {GetOwner()}, ProjectilePathResult);

	ResetSpline();
	DrawSpline(ProjectilePathResult);

	TensionPercent = (ArrowParams.Speed - ArrowCDO->GetMinSpeed()) / (ArrowCDO->GetMaxSpeed() - ArrowCDO->GetMinSpeed());
	
	//if(GameHUDWidget) GameHUDWidget->GetBowPowerWidget()->SetPower(ArrowParams.Speed, ArrowCDO->GetMinSpeed(), ArrowCDO->GetMaxSpeed());

	Server_FireButtonHolding(TensionPercent);
}

void UBowComponent::Server_FireButtonHolding_Implementation(float InTensionPercent)
{
	TensionPercent = InTensionPercent;
	Pawn->GetArrowFromQuiverMesh()->SetVisibility(false);
	Pawn->GetArrowOnBowTension()->SetVisibility(true);
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

void UBowComponent::ChangeArrow(Arrow::EType ArrowType)
{
	bChangeArrow = true;
	if(const TSubclassOf<ABasicArrowProjectile> ArrowResult = ArrowTypes.FindChecked(ArrowType))
	{
		if(ArrowResult == CurrentArrow)
		{
			Server_ChangeArrow(CurrentArrow, ArrowCDO->GetMesh());
			return;
		}
		CurrentArrow = ArrowResult;
		ArrowCDO = Cast<ABasicArrowProjectile>(ArrowResult->ClassDefaultObject);
		Pawn->GetArrowOnBowTension()->SetStaticMesh(ArrowCDO->GetMesh());
		Pawn->GetArrowFromQuiverMesh()->SetStaticMesh(ArrowCDO->GetMesh());
		Server_ChangeArrow(CurrentArrow, ArrowCDO->GetMesh());
	}
}

void UBowComponent::Server_ChangeArrow_Implementation(TSubclassOf<ABasicArrowProjectile> InCurrentArrow, UStaticMesh* ArrowMesh)
{
	bChangeArrow = true;
	if(InCurrentArrow == CurrentArrow) return;
	CurrentArrow = InCurrentArrow;
	Pawn->GetArrowOnBowTension()->SetStaticMesh(ArrowMesh);
	Pawn->GetArrowFromQuiverMesh()->SetStaticMesh(ArrowMesh);
}

void UBowComponent::OnChangeArrowFinished()
{
	bChangeArrow = false;
	Server_OnChangeArrowFinished();
}

void UBowComponent::Server_OnChangeArrowFinished_Implementation()
{
	bChangeArrow = false;
}

void UBowComponent::OnGetArrowFromQuiver()
{
	Pawn->GetArrowFromQuiverMesh()->SetVisibility(true);

	Server_OnGetArrowFromQuiver();
}

void UBowComponent::Server_OnGetArrowFromQuiver_Implementation()
{
	Pawn->GetArrowFromQuiverMesh()->SetVisibility(true);
}

void UBowComponent::Fire(const FInputActionInstance& ActionInstance)
{
	if(!bBowTensionIdle) return;

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
		
		GetWorld()->SpawnActor(CurrentArrow, &SpawnTransform, ActorSpawnParameters);
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
	
	GetWorld()->SpawnActor(CurrentArrow, &SpawnTransform, ActorSpawnParameters);
}
