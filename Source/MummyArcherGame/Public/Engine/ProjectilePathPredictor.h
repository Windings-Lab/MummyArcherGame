// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "ProjectilePathPredictor.generated.h"

/*
T		- Time
V		- Velocity
TH		- TraceHit
DT		- DeltaTime
*/

USTRUCT()
struct FWindPredictionResult
{
	GENERATED_BODY()

	class AWindArea* WindArea;
	bool bDetected;
	FVector Acceleration;
	float Time;
};

/**
 * 
 */
UCLASS()
class MUMMYARCHERGAME_API UProjectilePathPredictor : public UObject
{
	GENERATED_BODY()

	// Maximum simulation time for the virtual projectile.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Time", meta = (AllowPrivateAccess = "true"))
		float MaxSimulationT;
	// Determines size of each sub-step in the simulation (chopping up MaxSimTime). Recommended between 10 to 30 depending on desired quality versus performance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Time", meta = (AllowPrivateAccess = "true"))
		float SimulationFrequency;

	// Debug drawing duration option.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta = (AllowPrivateAccess = "true"))
		TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType;
	// Debug drawing duration option.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta = (AllowPrivateAccess = "true"))
		float DrawDebugTime;
	// Projectile radius, used when tracing for collision. If <= 0, a line trace is used instead.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta = (AllowPrivateAccess = "true"))
		float ProjectileRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug", meta = (AllowPrivateAccess = "true"))
		bool bDrawArc;

	// Whether or not to use TraceChannel, if tracing with collision.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision", meta = (AllowPrivateAccess = "true"))
		bool bTraceWithCollision;
	// Trace against complex collision (triangles rather than simple primitives) if tracing with collision.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision", meta = (AllowPrivateAccess = "true"))
		bool bTraceComplex;

	// Whether to trace along the path looking for blocking collision and stopping at the first hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Channel", meta = (AllowPrivateAccess = "true"))
		bool bTraceWithChannel;
	// Whether to trace along the path looking for blocking collision and stopping at the first hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Channel", meta = (AllowPrivateAccess = "true"))
		bool bTraceWithWindDetection;
	// Trace channel to use, if tracing with collision.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Channel", meta = (AllowPrivateAccess = "true"))
		TEnumAsByte<ECollisionChannel> TraceChannel;
	// Object type to use, if tracing with collision.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Channel", meta = (AllowPrivateAccess = "true"))
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

public:
	UProjectilePathPredictor();

	/**
	 * @return Initial Direction
	 */
	void PredictProjectilePathWithWind(const UWorld& World
		, const struct FProjectileParams& ProjectileParams
		, const TArray<AActor*>& ActorsToIgnore
		, FPredictProjectilePathResult& PredictResult);

	FVector GetInitialArrowDirection(const UWorld& World, const FProjectileParams& ProjectileParams, const TArray<AActor*>& ActorsToIgnore) const;

	static FWindPredictionResult DetectWindEnter(const UWorld* InWorld
	, const FProjectileBounds& InBounds
	, const FProjectileBounds& InOldBounds
	, const TArray<AActor*>& ActorsToIgnore);
	static FWindPredictionResult DetectWindExit(const UWorld* InWorld
		, const FProjectileBounds& InBounds
		, const FProjectileBounds& InOldBounds
		, const TArray<AActor*>& ActorsToIgnore);

private:
	void InitClassParams(const FProjectileParams& ProjectileParams);

	void Predict(const UWorld& World, const TArray<AActor*>& ActorsToIgnore, FPredictProjectilePathResult& PredictResult);
	
	bool DetectCollision(const UWorld& World, const FProjectileBounds& OldBounds, const TArray<AActor*>& ActorsToIgnore);

	void DrawPath(const UWorld& World, FPredictProjectilePathResult& PredictResult);

private:
	FORCEINLINE void UpdateTime();
	FORCEINLINE FVector MoveDeltaFormula(const FVector& Velocity, const FVector& OldVelocity, float DeltaTime);

private:
	// TH - TraceHit
	FHitResult ObjectTH;
	// TH - TraceHit
	FHitResult ChannelTH;
	
	TSet<AActor*>	   WindToIgnoreFromStart;
	TSet<AActor*>	   WindToIgnoreFromEnd;

	// T - Time
	float SimulationT = 0.f;
	// DT - Delta Time
	float SubstepDT = 0.f;
	// DT - Delta Time
	float DT = 0.f;

	// V - Velocity
	FVector V;
	// A - Acceleration
	FVector A;
	FVector MoveDelta;
	
	FProjectileBounds Bounds;
};
