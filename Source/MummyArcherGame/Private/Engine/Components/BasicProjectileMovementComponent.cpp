#include "Engine/Components/BasicProjectileMovementComponent.h"

#include "Engine/ProjectilePathPredictor.h"
#include "GameRules/MummyGameState.h"

#define OBJECTTYPE_WINDAREA ECC_GameTraceChannel3

UBasicProjectileMovementComponent::UBasicProjectileMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	OverridedMoveDelta = FVector::Zero();
	OverridedNewVelocity = FVector::Zero();
}

void UBasicProjectileMovementComponent::SetWindModificator(const FVector& Vector)
{
	IAffectedByWind::SetWindModificator(Vector);

	WindModificator += Vector;
}

void UBasicProjectileMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UBasicProjectileMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                      FActorComponentTickFunction* ThisTickFunction)
{
	// TODO: Fix formulas, it's not working in current situation. Related to ProjectilePathPredictor
#if 0
	auto* ArrowProjectile = Cast<ABasicArrowProjectile>(GetOwner());
	if(!ArrowProjectile)
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		return;
	}

	FVector MoveD = Super::ComputeMoveDelta(Velocity, DeltaTime);
	FProjectileBounds OldBounds = ArrowProjectile->GetBounds();
	FProjectileBounds Bounds	= OldBounds + MoveD;

	const FWindPredictionResult WindEnterResult = UProjectilePathPredictor::DetectWindEnter(GetWorld(), Bounds, OldBounds, {});
	const FWindPredictionResult WindExitResult  = UProjectilePathPredictor::DetectWindExit(GetWorld(), Bounds, OldBounds, {});
	
	if(WindEnterResult.bDetected && WindExitResult.bDetected)
	{
		// 1 - On Enter Wind Area
		float T1		= WindEnterResult.Time * DeltaTime;
		OverridedMoveDelta = Super::ComputeMoveDelta(Velocity, T1);
		FVector V1		= GetVelocityFromMoveDelta(OverridedMoveDelta, Velocity, T1);

		// 2 - Inside Wind Area
		float T2		= (1.f - WindEnterResult.Time - (1.f - WindExitResult.Time)) * DeltaTime;
		SetWindModificator(WindEnterResult.Acceleration);
		MoveD			= Super::ComputeMoveDelta(V1, T2);
		OverridedMoveDelta += MoveD;
		FVector V2		= GetVelocityFromMoveDelta(MoveD, V1, T2);

		// 3 - On Exit Wind Area
		float T3		= DeltaTime - T2 - T1;
		SetWindModificator(WindExitResult.Acceleration);
		MoveD			= Super::ComputeMoveDelta(V2, T3);
		OverridedMoveDelta += MoveD;
		FVector V3		= GetVelocityFromMoveDelta(MoveD, V2, T3);

		OverridedNewVelocity = V3;
	}
	else if(WindEnterResult.bDetected)
	{
		// 1 - On Enter Wind Area
		float T1		= WindEnterResult.Time * DeltaTime;
		OverridedMoveDelta = Super::ComputeMoveDelta(Velocity, T1);
		FVector V1		= GetVelocityFromMoveDelta(OverridedMoveDelta, Velocity, T1);
		
		// 2 - Inside Wind Area
		float T2		= DeltaTime - T1;
		SetWindModificator(WindEnterResult.Acceleration);
		MoveD			= Super::ComputeMoveDelta(V1, T2);
		OverridedMoveDelta += MoveD;
		FVector V2		= GetVelocityFromMoveDelta(MoveD, V1, T2);

		OverridedNewVelocity = V2;
	}
	else if(WindExitResult.bDetected)
	{
		// 1 - Inside Wind Area
		float T1		= WindExitResult.Time * DeltaTime;
		OverridedMoveDelta = Super::ComputeMoveDelta(Velocity, T1);
		FVector V1		= GetVelocityFromMoveDelta(OverridedMoveDelta, Velocity, T1);

		// 2 - On Exit Wind Area
		float T2		= DeltaTime - T1;
		SetWindModificator(WindExitResult.Acceleration);
		MoveD			= Super::ComputeMoveDelta(V1, T2);
		OverridedMoveDelta += MoveD;
		FVector V2 = GetVelocityFromMoveDelta(MoveD, V1, T2);

		OverridedNewVelocity = V2;
	}
#endif
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if constexpr (false)
	{
		OverridedMoveDelta = FVector::Zero();
		OverridedNewVelocity  = FVector::Zero();
	}
}

FVector UBasicProjectileMovementComponent::ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	AMummyGameState* GameState = GetWorld()->GetGameState<AMummyGameState>();
	FVector WindAcceleration = FVector::Zero();
	if(GameState)
	{
		WindAcceleration = GameState->GetWindAcceleration();
	}
	return Super::ComputeAcceleration(InVelocity, DeltaTime) + WindModificator + WindAcceleration;
}

FVector UBasicProjectileMovementComponent::ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const
{
	if(OverridedMoveDelta.IsZero())
	{
		return Super::ComputeMoveDelta(InVelocity, DeltaTime);
	}
	
	return OverridedMoveDelta;
}

FVector UBasicProjectileMovementComponent::ComputeVelocity(FVector InitialVelocity, float DeltaTime) const
{
	if(OverridedNewVelocity.IsZero())
	{
		return Super::ComputeVelocity(InitialVelocity, DeltaTime);
	}

	return LimitVelocity(OverridedNewVelocity);
}

FORCEINLINE FVector UBasicProjectileMovementComponent::GetVelocityFromMoveDelta(const FVector& MoveDelta, const FVector& V0, float DT)
{
	return ((MoveDelta - V0 * DT) / (0.5f * DT)) + V0;
}
