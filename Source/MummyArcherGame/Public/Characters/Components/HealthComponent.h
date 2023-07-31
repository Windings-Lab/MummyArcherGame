// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
		int32 GetHealth() { return Health; }

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
		
};
