// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BasicCharacter.generated.h"

UCLASS()
class MUMMYARCHERGAME_API ABasicCharacter : public ACharacter
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

public:
	ABasicCharacter();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FVector GetAimLocation() const { return AimLocation; }

protected:
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

private:
	UFUNCTION()
		virtual void Look(const struct FInputActionValue& Value);
	
	FVector TraceLine(bool DrawTrace, FHitResult& HitResult);

private:
	FHitResult AimHitResult;
	FVector AimLocation;
};
