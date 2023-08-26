// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/MummyCharacter.h"

#include "EnhancedInputComponent.h"
#include "Characters/Components/BowComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "Characters/Components/QuiverComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

AMummyCharacter::AMummyCharacter()
{
	SkeletalBow = CreateDefaultSubobject<UBowComponent>(TEXT("SkeletalBow"));
	SkeletalBow->SetupAttachment(GetMesh(), TEXT("bow_socket"));
	
	Quiver = CreateDefaultSubobject<UQuiverComponent>(TEXT("Quiver"));
	Quiver->SetupAttachment(GetMesh());
}

void AMummyCharacter::ChangeArrow(TEnumAsByte<Arrow::EType> ArrowType)
{
	SkeletalBow->SetArrowType(ArrowType);
	SkeletalBow->SetArrow(Quiver->GetArrow(ArrowType));
}

void AMummyCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->HideBoneByName(TEXT("Bow"), PBO_None);
	
	APlayerController* PlayerController = GetController<APlayerController>();
	if(!PlayerController) return;

	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!Subsystem) return;
	
	SkeletalBow->AddBowMappingContext(Subsystem, 1);


	if(IsLocallyControlled())
	{
		ChangeArrow(Arrow::EType::Basic);
	}
}

void AMummyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	APlayerController* PlayerController = GetController<APlayerController>();
	if(!PlayerController) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!Subsystem) return;
	
	SkeletalBow->RemoveBowMappingContext(Subsystem);
}

void AMummyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	SkeletalBow->SetupPlayerInput(PlayerInputComponent);

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
