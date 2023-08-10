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
	class UWidgetComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UHealthComponent* Health;
	
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
	void Hit(int Damage);

	UFUNCTION(BlueprintCallable)
	void Heal(int Recovery);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE	UWidgetComponent* GetHealthBarWidget() const { return HealthBarWidget; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FVector GetAimLocation() const { return AimLocation; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetArrowHitNormal() const { return ArrowHitNormal; }

	UFUNCTION(BlueprintCallable)
		FRotator GetAimOffset();

public:
	// Animation Properties
	UPROPERTY(BlueprintReadWrite)
	bool bOnHit = false;

	UPROPERTY(Replicated)
	FRotator AimOffset;

	UPROPERTY(Replicated)
		FHitResult AimHitResult;
	UPROPERTY(Replicated)
		FVector AimLocation;

protected:
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
		virtual void Look(const struct FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
		void Server_UpdateAimOffset(const FRotator& InAimOffset);

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	FVector TraceLine(bool DrawTrace, FHitResult& HitResult);

private:
	float ArrowHitNormal;
};
