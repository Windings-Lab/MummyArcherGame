// Fill out your copyright notice in the Description page of Project Settings.


#include "AbstractClasses/Characters/BasicCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ABasicCharacter::ABasicCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	auto* CharacterMesh = GetMesh();
	
	CharacterMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	CharacterMesh->SetRelativeRotation(FRotator(0.f, 270.f, 0.f));
	
	auto* CharacterMove = GetCharacterMovement();
	CharacterMove->bOrientRotationToMovement = true;	
	CharacterMove->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	
	CharacterMove->JumpZVelocity = 700.f;
	CharacterMove->AirControl = 0.35f;
	CharacterMove->MaxWalkSpeed = 500.f;
	CharacterMove->MinAnalogWalkSpeed = 20.f;
	CharacterMove->BrakingDecelerationWalking = 2000.f;

	CharacterMove->GravityScale = 1.75f;
	CharacterMove->MaxAcceleration = 1500.f;
	CharacterMove->BrakingFrictionFactor = 1.f;
	CharacterMove->bUseSeparateBrakingFriction = true;

	CharacterMove->SetFixedBrakingDistance(200.f);
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ABasicCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetController<APlayerController>();
	if(!PlayerController) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if(!Subsystem) return;
	
	Subsystem->AddMappingContext(DefaultMappingContext, 0);
}

void ABasicCharacter::Look(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;
	
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}


void ABasicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent) return;

	//Looking
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasicCharacter::Look);
}

