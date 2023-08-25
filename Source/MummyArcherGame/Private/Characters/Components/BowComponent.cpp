// Copyright Epic Games, Inc. All Rights Reserved.


#include "Characters/Components/BowComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Characters/MummyCharacter.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/ProjectilePathPredictor.h"
#include "Engine/Components/BasicProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/MummyHUD.h"

UBowComponent::UBowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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

void UBowComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UBowComponent, bFocused, COND_SkipOwner)
	DOREPLIFETIME_CONDITION(UBowComponent, bTransitionToBowTensionIdle, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UBowComponent, bTransitionToFocusIdle, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UBowComponent, TensionPercent, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UBowComponent, bTransitionToChangeArrow, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(UBowComponent, CurrentArrowType, COND_SkipOwner);

	DOREPLIFETIME(UBowComponent, CurrentArrow);
}

void UBowComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		/*EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Started, this, &UBowComponent::FireButtonPressed);
		EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Triggered, this, &UBowComponent::Fire);
		EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Completed, this, &UBowComponent::FireButtonReleased);
		EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Canceled, this, &UBowComponent::FireButtonReleased);
		EnhancedInputComponent->BindAction(BowFireAction, ETriggerEvent::Ongoing, this, &UBowComponent::FireButtonHolding);*/
			
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

	Server_Focus(bFocused, bTransitionToFocusIdle);
}

void UBowComponent::Focus()
{
	bFocused = true;
	
	auto* Camera = Pawn->GetFollowCamera();
	Camera->SetFieldOfView(30.f);
	bTransitionToFocusIdle = true;
}

void UBowComponent::Unfocus()
{
	bFocused = false;
	
	auto* Camera = Pawn->GetFollowCamera();
	Camera->SetFieldOfView(90.f);
	if(!bTransitionToBowTensionIdle)
	{
		bTransitionToFocusIdle = false;
		TimerBeforeGetArrow = 0.f;
	}
}

void UBowComponent::Server_Focus_Implementation(bool InFocused, bool InBowTensionIdle)
{
	bFocused = InFocused;
	bTransitionToFocusIdle = InBowTensionIdle;
}

void UBowComponent::FireButtonPressed()
{
	if(!CurrentArrow) return;
	//if(GameHUDWidget) GameHUDWidget->ShowBowPower();
	bTransitionToFocusIdle = true;
	bTransitionToBowTensionIdle	= true;
	
	Server_FireButtonPressed();
}

void UBowComponent::Server_FireButtonPressed_Implementation()
{
	bTransitionToFocusIdle = true;
	bTransitionToBowTensionIdle	= true;
}

void UBowComponent::FireButtonReleased()
{
	if(!CurrentArrow) return;
	//if(GameHUDWidget) GameHUDWidget->HideBowPower();
	
	ResetSpline();
	ArcEndSphere->SetVisibility(false, false);

	bTransitionToBowTensionIdle = false;
	bTransitionToChangeArrow = false;
	TimerBeforeGetArrow = 0.f;
	TensionPercent = 0.f;
	if(!bFocused)
	{
		bTransitionToFocusIdle = false;
	}
	
	Server_FireButtonReleased(bTransitionToFocusIdle);
}

void UBowComponent::Server_FireButtonReleased_Implementation(bool InBowTensionIdle)
{
	bTransitionToBowTensionIdle = false;
	bTransitionToChangeArrow = false;
	bTransitionToFocusIdle = InBowTensionIdle;
	TensionPercent = 0.f;
}

void UBowComponent::OnInterrupted()
{
	if(Pawn->IsLocallyControlled())
	{
		Unfocus();
		ResetSpline();
		ArcEndSphere->SetVisibility(false, false);
		TimerBeforeGetArrow = 0.f;
		TimerOnFirePressed = 0.f;
	}

	CurrentArrow->GetRootComponent()->SetVisibility(false, true);

	bTransitionToBowTensionIdle = false;
	bTransitionToChangeArrow = false;
	TensionPercent = 0.f;
	bTransitionToFocusIdle = false;
}

void UBowComponent::OnRep_CurrentArrow()
{
	if(!CurrentArrow) return;
	bTransitionToChangeArrow = true;

	if(Pawn->IsLocallyControlled())
	{
		bChangingArrow = false;
	}
}

FProjectileParams UBowComponent::CreateArrowParams(float BowTensionTime)
{
	const FVector Acceleration = FVector(0.f, 0.f, -980 * CurrentArrow->GetGravityScale());
	
	FProjectileParams ArrowParameters(GetSocketTransform(TEXT("arrow_socket"))
		, CurrentArrow->GetBounds()
		, Pawn->GetAimLocation()
		, CurrentArrow->CalculateArrowSpeed(BowTensionTime, MaxBowTensionTime)
		, CurrentArrow->GetMaxSpeed()
		, Acceleration);

	const FVector Direction = ArrowPathPredictor->GetInitialArrowDirection(*GetWorld(), ArrowParameters, {GetOwner()});
	ArrowParameters.Transform.SetRotation(Direction.ToOrientationQuat());
	ArrowParameters.Bounds = CurrentArrow->GetBounds(GetSocketLocation(TEXT("arrow_socket")), Direction);

	return ArrowParameters;
}

void UBowComponent::FireButtonHolding(const FInputActionInstance& ActionInstance)
{
	if(!CurrentArrow) return;
	
	if(!bInBowTensionIdleState)
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

	TensionPercent = (ArrowParams.Speed - CurrentArrow->GetMinSpeed()) / (CurrentArrow->GetMaxSpeed() - CurrentArrow->GetMinSpeed());
	
	//if(GameHUDWidget) GameHUDWidget->GetBowPowerWidget()->SetPower(ArrowParams.Speed, ArrowCDO->GetMinSpeed(), ArrowCDO->GetMaxSpeed());

	Server_FireButtonHolding(TensionPercent);
}

void UBowComponent::Server_FireButtonHolding_Implementation(float InTensionPercent)
{
	TensionPercent = InTensionPercent;
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

void UBowComponent::OnFirePressed()
{
	FireButtonPressed();
}

void UBowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(Pawn && Pawn->IsLocallyControlled())
	{
		if(!CurrentArrow) return;

		if(bTransitionToBowTensionIdle) TimerOnFirePressed += DeltaTime;
		else return;
		
		if(!bInBowTensionIdleState)
		{
			TimerOnFirePressed = 0.f;
			return;
		}
		
		FProjectileParams ArrowParams = CreateArrowParams(TimerOnFirePressed);

		FPredictProjectilePathResult ProjectilePathResult;
		ArrowPathPredictor->PredictProjectilePathWithWind(*GetWorld(), ArrowParams, {GetOwner()}, ProjectilePathResult);

		ResetSpline();
		DrawSpline(ProjectilePathResult);

		TensionPercent = (ArrowParams.Speed - CurrentArrow->GetMinSpeed()) / (CurrentArrow->GetMaxSpeed() - CurrentArrow->GetMinSpeed());

		Server_FireButtonHolding(TensionPercent);
	}
}

void UBowComponent::OnFireReleased()
{
	FireButtonReleased();

	if(!bInBowTensionIdleState || bChangingArrow)
	{
		TimerOnFirePressed = 0.f;
		return;
	}
	
	FProjectileParams ArrowParams = CreateArrowParams(TimerOnFirePressed);

	FPredictProjectilePathResult ProjectilePathResult;
	ArrowPathPredictor->PredictProjectilePathWithWind(*GetWorld(), ArrowParams, {GetOwner()}, ProjectilePathResult);
	
	Fire(ArrowParams.Transform, ArrowParams.Speed);
	TimerOnFirePressed = 0.f;
}

void UBowComponent::SetArrow(TSubclassOf<class ABasicArrowProjectile> InCurrentArrow)
{
	TensionPercent = 0.f;
	bChangingArrow = true;
	
	ResetSpline();
	ArcEndSphere->SetVisibility(false, false);
	
	Server_SetArrow(InCurrentArrow);
}

void UBowComponent::SetArrowType(TEnumAsByte<Arrow::EType> ArrowType)
{
	if(CurrentArrowType == ArrowType) return;
	
	CurrentArrowType = ArrowType;
	Server_SetArrowType(ArrowType);
}

void UBowComponent::Server_SetArrowType_Implementation(Arrow::EType ArrowType)
{
	CurrentArrowType = ArrowType;
}

void UBowComponent::Server_SetArrow_Implementation(TSubclassOf<class ABasicArrowProjectile> InCurrentArrow)
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = Pawn;
	ActorSpawnParameters.Instigator = Pawn;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if(CurrentArrow) CurrentArrow->Destroy();

	CurrentArrow = Cast<ABasicArrowProjectile>(GetWorld()->SpawnActor(InCurrentArrow, &Pawn->GetMesh()->GetComponentTransform(), ActorSpawnParameters));
	CurrentArrow->SetIgnoredActor(Pawn);
}

void UBowComponent::OnReturnToIdleState()
{
	CurrentArrow->GetRootComponent()->SetVisibility(false, true);
}

void UBowComponent::OnChangeArrowFinished()
{
	bTransitionToChangeArrow = false;
}

void UBowComponent::OnBowTensionIdleState(bool InState)
{
	bInBowTensionIdleState = InState;

	if(Pawn->HasAuthority()) return;
	if(bInBowTensionIdleState)
	{
		CurrentArrow->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale, Pawn->GetArrowSocket());
	}
}

void UBowComponent::OnGetArrowFromQuiver()
{
	CurrentArrow->GetRootComponent()->SetVisibility(true, true);
	
	if(Pawn->HasAuthority()) return;
	CurrentArrow->AttachToComponent(Pawn->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, Pawn->GetQuiverSocket());
}

void UBowComponent::Fire(const FInputActionInstance& ActionInstance)
{
	if(!bInBowTensionIdleState || bChangingArrow) return;

	float TimerDelta = ActionInstance.GetElapsedTime() - TimerBeforeGetArrow;
	FProjectileParams ArrowParams = CreateArrowParams(TimerDelta);

	FPredictProjectilePathResult ProjectilePathResult;
	ArrowPathPredictor->PredictProjectilePathWithWind(*GetWorld(), ArrowParams, {GetOwner()}, ProjectilePathResult);

	Fire(ArrowParams.Transform, ArrowParams.Speed);
}

void UBowComponent::Fire(const FTransform& InTransform, float Speed)
{
	Server_Fire(InTransform, Speed);
	CurrentArrow = nullptr;
	Pawn->ChangeArrow(CurrentArrowType);
}

void UBowComponent::Server_Fire_Implementation(const FTransform& InTransform, float Speed)
{
	CurrentArrow->Server_Fire(InTransform, Speed);
	CurrentArrow = nullptr;
}
