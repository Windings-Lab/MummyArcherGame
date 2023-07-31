// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbstractClasses/Characters/BasicCharacter.h"
#include "AbstractClasses/Characters/Interfaces/CanJump.h"
#include "AbstractClasses/Characters/Interfaces/CanMove.h"
#include "MummyCharacter.generated.h"

UCLASS()
class MUMMYARCHERGAME_API AMummyCharacter : public ABasicCharacter, public ICanMove, public ICanJump
{
	GENERATED_BODY()

public:
	AMummyCharacter();

	UFUNCTION(BlueprintCallable)
		void Hit(int Damage);

	UFUNCTION(BlueprintCallable)
		void Heal(int Recovery);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UBowComponent* Bow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
		class UHealthComponent* Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
		UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
		class UInputAction* JumpAction;


	UFUNCTION()
		virtual void Move(const FInputActionValue& Value) override;
	UFUNCTION()
		virtual UInputAction* GetMoveAction() override;
	UFUNCTION()
		virtual UInputAction* GetJumpAction() override;

};
