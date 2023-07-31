// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/MummyCharacter.h"

#include "EnhancedInputComponent.h"
#include "Characters/Components/BowComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Characters/Components/HealthComponent.h"
#include "Characters/Controllers/MummyPlayerController.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "UI/MummyHUD.h"

AMummyCharacter::AMummyCharacter()
{
	Bow = CreateDefaultSubobject<UBowComponent>(TEXT("Bow"));
	Bow->SetupAttachment(GetFollowCamera());

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
}

void AMummyCharacter::Hit(int Damage)
{
	Health->Hit(Damage);
}

void AMummyCharacter::Heal(int Recovery)
{
	Health->Heal(Recovery);
}

void AMummyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PlayerController = GetController<APlayerController>();
	if(!PlayerController) return;

	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!Subsystem) return;
	
	Bow->AddBowMappingContext(Subsystem, 1);
	Hit(0);
		
}

void AMummyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	APlayerController* PlayerController = GetController<APlayerController>();
	if(!PlayerController) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!Subsystem) return;
	
	Bow->RemoveBowMappingContext(Subsystem);
}

void AMummyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	Bow->SetupPlayerInput(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent) return;

	//Jumping
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

	//Moving
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMummyCharacter::Move);
}

void AMummyCharacter::Move(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
		
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

UInputAction* AMummyCharacter::GetMoveAction()
{
	return MoveAction;
}

UInputAction* AMummyCharacter::GetJumpAction()
{
	return JumpAction;
}
