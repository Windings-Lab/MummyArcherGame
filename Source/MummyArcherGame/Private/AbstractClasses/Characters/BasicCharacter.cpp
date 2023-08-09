// Fill out your copyright notice in the Description page of Project Settings.


#include "AbstractClasses/Characters/BasicCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbstractClasses/Arrow/BasicArrowProjectile.h"
#include "Camera/CameraComponent.h"
#include "Characters/Components/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#define TRACE_CHARACTER ECC_GameTraceChannel2

// Sets default values
ABasicCharacter::ABasicCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	auto* CharacterMesh = GetMesh();

	CharacterMesh->OnComponentHit.AddDynamic(this, &ABasicCharacter::OnHit);
	
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

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>((TEXT("Widget")));
	HealthBarWidget->SetupAttachment(RootComponent);
}

void ABasicCharacter::Hit(int Damage)
{
	Health->Hit(Damage);
	bOnHit = true;
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


FVector ABasicCharacter::TraceLine(bool DrawTrace, FHitResult& HitResult)
{
	const auto* Camera = GetFollowCamera();
	const FVector CameraForwardVector = Camera->GetForwardVector();
	const FVector CameraLocation = Camera->GetComponentLocation();

	const FVector TraceStartLocation = UKismetMathLibrary::FindClosestPointOnLine(GetActorLocation(), CameraLocation, CameraForwardVector);
	const FVector TraceEndLocation = TraceStartLocation + CameraForwardVector * 10000.f;
	const EDrawDebugTrace::Type DrawDebugTraceType = DrawTrace ? EDrawDebugTrace::Type::ForDuration : EDrawDebugTrace::Type::None;
	
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);

	UKismetSystemLibrary::LineTraceSingle(GetWorld()
		, TraceStartLocation, TraceEndLocation
		, UEngineTypes::ConvertToTraceType(TRACE_CHARACTER)
		, false
		, {this}
		, DrawDebugTraceType
		, HitResult
		, false
		, FLinearColor::Green
		, FLinearColor::Red
		, 0.f);

	return HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceEndLocation;
}

void ABasicCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AimLocation = TraceLine(false, AimHitResult);
}

void ABasicCharacter::Look(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;
	
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ABasicCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor->IsA<ABasicArrowProjectile>())
	{
		ArrowHitNormal = UKismetMathLibrary::MakeRelativeTransform(OtherActor->GetActorTransform(), GetActorTransform()).GetRotation().Z;
	}
}

void ABasicCharacter::Heal(int Recovery)
{
	Health->Heal(Recovery);
}

void ABasicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent) return;

	//Looking
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasicCharacter::Look);
}

