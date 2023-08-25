// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbstractClasses/Characters/BasicCharacter.h"
#include "AbstractClasses/Characters/Interfaces/CanJump.h"
#include "AbstractClasses/Characters/Interfaces/CanMove.h"
#include "MummyCharacter.generated.h"

namespace Arrow
{
	enum EType : int;
}

UCLASS()
class MUMMYARCHERGAME_API AMummyCharacter : public ABasicCharacter, public ICanMove, public ICanJump
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 TeamNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 PositionInTeam = 0;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UBowComponent* SkeletalBow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UQuiverComponent* Quiver;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

public:
	AMummyCharacter();

	UFUNCTION(BlueprintCallable)
		FORCEINLINE	UBowComponent* GetBowComponent() const { return SkeletalBow; }
	UFUNCTION(BlueprintCallable)
		void ChangeArrow(TEnumAsByte<Arrow::EType> ArrowType);

	FORCEINLINE FName GetArrowSocket() const { return TEXT("arrow_socket"); }
	FORCEINLINE FName GetQuiverSocket() const { return TEXT("getArrow_socket"); }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UFUNCTION()
		virtual void Move(const FInputActionValue& Value) override;
	UFUNCTION()
		virtual UInputAction* GetMoveAction() override;
	UFUNCTION()
		virtual UInputAction* GetJumpAction() override;

};
