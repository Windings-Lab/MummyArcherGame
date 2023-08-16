// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MummyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MUMMYARCHERGAME_API AMummyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	//UFUNCTION(BlueprintCallable)
	//void OnKilled();

	//void Respawn();

	UFUNCTION(Server, Reliable)
	void SetTeamNumberOnServer(int32 TeamNumberVar);

	UFUNCTION(Server, Reliable)
	void SetPositionInTeamOnServer(int32 PositionInTeamVar);


	UFUNCTION(Server, Reliable)
		void AddWidgetOnServer();

	UFUNCTION(Server, Reliable)
	void AddWidgetOnClient(const FString& Message);



protected:
	
	FTimerHandle TimerHandleRespawn;

	float InRate = 5.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 TeamNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 PositionInTeam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UUserWidget* Widget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UUserWidget> WidgetClass;
};
