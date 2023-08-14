// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <GameRules/MummyGameState.h>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GameHUDWidget.h"
#include "HealthComponent.generated.h"

class UHealthWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MUMMYARCHERGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UFUNCTION(BlueprintCallable)
		void Hit(int32 Parameter);

	UFUNCTION(BlueprintCallable)
		void Heal(int32 Parameter);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE	int32 GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE	float GetPercent() const
	{ return static_cast<float>(Health) / static_cast<float>(DefaultHealth); }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE	bool IsDead() const
	{	return bIsDead;	}

	UFUNCTION(BlueprintCallable)
		void Kill()
	{
		bIsDead = true;
		Cast<AMummyGameState>(UGameplayStatics::GetGameState(GetWorld()))->OnPlayerKill();
		
	}

protected:
	virtual void InitializeComponent() override;

	void ChangeHealth(int32 Parameter);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Settings")
		int32 DefaultHealth = 100;

private:
	int32 Health;

	UPROPERTY()
		TObjectPtr<UGameHUDWidget> GameHUDWidget;

	UPROPERTY()
		TObjectPtr<UHealthWidget> HealthWidget;

	bool bIsDead = false;
		
};
