// Fill out your copyright notice in the Description page of Project Settings.


#include "Engine/ProjectilePathPredictor.h"

#include "Gameplay/WindArea.h"
#include "Kismet/GameplayStatics.h"

#define TRACE_CHARACTER ECC_GameTraceChannel2
#define OBJECTTYPE_WINDAREA ECC_GameTraceChannel3

UProjectilePathPredictor::UProjectilePathPredictor()
	: ObjectTH(NoInit)
	, ChannelTH(NoInit)
{
}

FVector UProjectilePathPredictor::GetInitialArrowDirection(const UWorld& World, const FProjectileParams& ProjectileParams, const TArray<AActor*>& ActorsToIgnore) const
{
	FVector Direction = ProjectileParams.ImpactPoint - ProjectileParams.Transform.GetLocation();

	if(ProjectileParams.Acceleration.Z == 0.f) return Direction.GetSafeNormal();

	FCollisionResponseParams CollisionResponseParams;
	CollisionResponseParams.CollisionResponse.SetResponse(TRACE_CHARACTER, ECR_Block);
	UGameplayStatics::SuggestProjectileVelocity(&World
		, Direction
		, ProjectileParams.Transform.GetLocation()
		, ProjectileParams.ImpactPoint
		, ProjectileParams.MaxSpeed
		, false
		, 2
		, ProjectileParams.Acceleration.Z
		, ESuggestProjVelocityTraceOption::Type::DoNotTrace
		, CollisionResponseParams
		, ActorsToIgnore
		, bDrawArc);

	return Direction.GetSafeNormal();
}

void UProjectilePathPredictor::PredictProjectilePathWithWind(const UWorld& World
	, const FProjectileParams& ProjectileParams
	, const TArray<AActor*>& ActorsToIgnore
	, FPredictProjectilePathResult& PredictResult)
{
	PredictResult.Reset();

	if (SimulationFrequency <= UE_KINDA_SMALL_NUMBER) return;

	PredictResult.HitResult.bBlockingHit = false;
	
	InitClassParams(ProjectileParams);

	Predict(World, ActorsToIgnore, PredictResult);

	DrawPath(World, PredictResult);
}

void UProjectilePathPredictor::InitClassParams(const FProjectileParams& ProjectileParams)
{
	A	   = ProjectileParams.Acceleration;
	V	   = ProjectileParams.Transform.GetRotation().GetForwardVector() * ProjectileParams.Speed;
	Bounds = ProjectileParams.Bounds;

	SimulationT			   = 0.f;
	SubstepDT	   = 1.f / SimulationFrequency;
	ObjectTH.Time  = 1.f;
	ChannelTH.Time = 1.f;
}

void UProjectilePathPredictor::Predict(const UWorld& World, const TArray<AActor*>& ActorsToIgnore, FPredictProjectilePathResult& PredictResult)
{
	PredictResult.PathData.Reserve(FMath::Min(128, FMath::CeilToInt(MaxSimulationT * SimulationFrequency)));
	PredictResult.AddPoint(Bounds.Min, V, 0.f);

	float   OldT;
	FVector OldV;
	FVector OldA;
	FProjectileBounds OldBounds;
	while (SimulationT < MaxSimulationT)
	{
		OldT	  = SimulationT;
		OldV	  = V;
		OldA	  = A;
		OldBounds = Bounds;
		
		UpdateTime();
		
		V		  = OldV + (A * DT);
		MoveDelta = MoveDeltaFormula(V, OldV, DT);
		Bounds	  = OldBounds + MoveDelta;

		// TODO: Fix formulas, it's not working in current situation. Related to BasicProjectileMovementComponent
#if 0
		if(bTraceWithWindDetection)
		{
			// WindToIgnoreFromStart.IsEmpty() ? TArray<AActor*>() : WindToIgnoreFromStart.Array()
			// WindToIgnoreFromEnd.IsEmpty() ? TArray<AActor*>() : WindToIgnoreFromEnd.Array()

			const FWindPredictionResult WindEnterResult = DetectWindEnter(&World, Bounds, OldBounds, {});
			const FWindPredictionResult WindExitResult  = DetectWindExit(&World, Bounds, OldBounds, {});

			if(WindEnterResult.bDetected && WindExitResult.bDetected)
			{
				// 1 - On Enter Wind Area
				const float   T1 = WindEnterResult.Time * DT;
				const FVector V1 = OldV + (A * T1);
				MoveDelta		 = MoveDeltaFormula(V1, OldV, T1);

				// 2 - Inside Wind Area
				const float   T2 = (1.f - WindEnterResult.Time - (1.f - WindExitResult.Time)) * DT;
				const FVector V2 = V1 + ((A + WindEnterResult.Acceleration) * T2);
				MoveDelta		 += MoveDeltaFormula(V2, V1, T2);

				// 3 - On Exit Wind Area
				const float   T3 = DT - T1 - T2;
				const FVector V3 = V2 + (A * T3);
				MoveDelta		 += MoveDeltaFormula(V3, V2, T3);

				WindToIgnoreFromStart.Add(WindEnterResult.WindArea);
				WindToIgnoreFromEnd.Add(WindEnterResult.WindArea);
			}
			else if(WindEnterResult.bDetected)
			{
				// 1 - On Enter Wind Area
				const float   T1 = WindEnterResult.Time * DT;
				const FVector V1 = OldV + (A * T1);
				MoveDelta		 = MoveDeltaFormula(V1, OldV, T1);

				A += WindEnterResult.Acceleration;

				// 2 - Inside Wind Area
				const float   T2 = DT - T1;
				const FVector V2 = V1 + (A * T2);
				MoveDelta		 += MoveDeltaFormula(V2, V1, T2);

				WindToIgnoreFromStart.Add(WindEnterResult.WindArea);
				WindToIgnoreFromEnd.Remove(WindEnterResult.WindArea);
			}
			else if(WindExitResult.bDetected)
			{
				// 1 - Inside Wind Area
				const float   T1 = WindExitResult.Time * DT;
				const FVector V1 = OldV + (A * T1);
				MoveDelta		 = MoveDeltaFormula(V1, OldV, T1);

				A += WindExitResult.Acceleration;
			
				// 2 - On Exit Wind Area
				const float   T2 = DT - T1;
				const FVector V2 = V1 + (A * T2);
				MoveDelta		 += MoveDeltaFormula(V2, V1, T2);

				WindToIgnoreFromEnd.Add(WindExitResult.WindArea);
				WindToIgnoreFromStart.Remove(WindExitResult.WindArea);
			}

			Bounds = OldBounds + MoveDelta;
		}
#endif
		
		PredictResult.LastTraceDestination.Set(Bounds.Min, V, SimulationT);
			
		if (DetectCollision(World, OldBounds, ActorsToIgnore))
		{
			PredictResult.HitResult	= (ObjectTH.Time < ChannelTH.Time) ? ObjectTH : ChannelTH;

			const float   HitDT = DT * PredictResult.HitResult.Time;
			const FVector HitV  = OldV + (OldA * HitDT);
			SimulationT			= OldT + HitDT;
			
			PredictResult.AddPoint(PredictResult.HitResult.ImpactPoint, HitV, SimulationT);
			break;
		}

		PredictResult.AddPoint(Bounds.Min, V, SimulationT);
	}

	WindToIgnoreFromEnd.Empty();
	WindToIgnoreFromStart.Empty();
}

FORCEINLINE void UProjectilePathPredictor::UpdateTime()
{
	DT			= FMath::Min(MaxSimulationT - SimulationT, SubstepDT);
	SimulationT	+= DT;
}

FORCEINLINE FVector UProjectilePathPredictor::MoveDeltaFormula(const FVector& Velocity, const FVector& OldVelocity, float DeltaTime)
{
	// (OldVelocity + Velocity) * (0.5f * DeltaTime)
	// (OldVelocity * DeltaTime) + (Velocity - OldVelocity) * (0.5f * DeltaTime)
	return (OldVelocity + Velocity) * (0.5f * DeltaTime);
}

FWindPredictionResult UProjectilePathPredictor::DetectWindEnter(const UWorld* InWorld
		, const FProjectileBounds& InBounds
		, const FProjectileBounds& InOldBounds
		, const TArray<AActor*>& ActorsToIgnore)
{
	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::SphereTraceMultiForObjects(InWorld
								, InOldBounds.Min, InBounds.Min, 1.f
								, {UEngineTypes::ConvertToObjectType(OBJECTTYPE_WINDAREA)}
								, false
								, ActorsToIgnore
								, EDrawDebugTrace::None
								, HitResults
								, true);
	HitResults.RemoveAll([](FHitResult& HitResult)
	{
		return HitResult.bStartPenetrating;
	});

	FVector Acceleration = FVector::Zero();
	const bool bEntered	 = !HitResults.IsEmpty();
	if(!bEntered) return { nullptr, false, Acceleration, 1.f };

	const float EnteredT = HitResults.Last().Time;
	AWindArea* WindArea = Cast<AWindArea>(HitResults.Last().GetActor());
	if(WindArea)
	{
		Acceleration = WindArea->GetAcceleration();
	}
	else
	{
		return { nullptr, false, Acceleration, 1.f };
	}
	
	return { WindArea, true, Acceleration, EnteredT };
}

FWindPredictionResult UProjectilePathPredictor::DetectWindExit(const UWorld* InWorld
		, const FProjectileBounds& InBounds
		, const FProjectileBounds& InOldBounds
		, const TArray<AActor*>& ActorsToIgnore)
{
	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::SphereTraceMultiForObjects(InWorld
							, InBounds.Min, InOldBounds.Min, 1.f
							, {UEngineTypes::ConvertToObjectType(OBJECTTYPE_WINDAREA)}
							, false
							, ActorsToIgnore
							, EDrawDebugTrace::None
							, HitResults
							, true);
	HitResults.RemoveAll([](FHitResult& HitResult)
	{
		return HitResult.bStartPenetrating;
	});


	const bool bExited  = !HitResults.IsEmpty();
	FVector Acceleration = FVector::Zero();
	
	if(!bExited) return { nullptr, false, Acceleration, 1.f };
	
	const float ExitedT = 1.f - HitResults.Last().Time;
	if(FMath::IsNearlyZero(ExitedT, 1E-04)) return { nullptr, false, Acceleration, 1.f };
	
	AWindArea* WindArea = Cast<AWindArea>(HitResults.Last().GetActor());
	if(WindArea)
	{
		Acceleration = WindArea->GetAcceleration();
	}
	else
	{
		return { nullptr, false, Acceleration, 1.f };
	}

	return { WindArea, true, -Acceleration, ExitedT };
}

bool UProjectilePathPredictor::DetectCollision(const UWorld& World, const FProjectileBounds& OldBounds, const TArray<AActor*>& ActorsToIgnore)
{
	if(!bTraceWithCollision) return false;
	
	ObjectTH.bBlockingHit = false;
	ChannelTH.bBlockingHit = false;
	if (ObjectTypes.Num() > 0)
	{
		UKismetSystemLibrary::SphereTraceSingleForObjects(&World
			, OldBounds.Min, Bounds.Min, 1.f
			, ObjectTypes
			, bTraceComplex
			, ActorsToIgnore
			, EDrawDebugTrace::None
			, ObjectTH
			, true);
	}
	if (bTraceWithChannel)
	{
		UKismetSystemLibrary::SphereTraceSingle(&World
			, OldBounds.Min, Bounds.Min, 1.f
			, UEngineTypes::ConvertToTraceType(TraceChannel)
			, bTraceComplex
			, ActorsToIgnore
			, EDrawDebugTrace::None
			, ChannelTH
			, true);
	}

	return ObjectTH.bBlockingHit || ChannelTH.bBlockingHit;
}

void UProjectilePathPredictor::DrawPath(const UWorld& World, FPredictProjectilePathResult& PredictResult)
{
	// Draw debug path
	#if ENABLE_DRAW_DEBUG
	
	if (DrawDebugType == EDrawDebugTrace::None)	return;
	const bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
	const float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawDebugTime : 0.f;
	const float DrawRadius = (ProjectileRadius > 0.f) ? ProjectileRadius : 5.f;

	// draw the path
	for (const FPredictProjectilePathPointData& PathPt : PredictResult.PathData)
	{
		::DrawDebugSphere(&World, PathPt.Location, DrawRadius, 12, FColor::Green, bPersistent, LifeTime);
	}
	// draw the impact point
	if (PredictResult.HitResult.bBlockingHit)
	{
		::DrawDebugSphere(&World, PredictResult.HitResult.Location, DrawRadius + 1.0f, 12, FColor::Red, bPersistent, LifeTime);
	}
	
	#endif //ENABLE_DRAW_DEBUG
}

