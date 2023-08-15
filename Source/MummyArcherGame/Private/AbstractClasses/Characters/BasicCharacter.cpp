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
#include "Net/UnrealNetwork.h"

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

FVector ABasicCharacter::GetAimLocation()
{
	FHitResult HitResult;
	return TraceLine(false, HitResult);
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

void ABasicCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(IsLocallyControlled())
	{
		UpdateAim();
	}
}

void ABasicCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABasicCharacter, AimOffset, COND_SkipOwner);
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

FRotator ABasicCharacter::RInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed)
{
	double ClampTargetYaw = FMath::ClampAngle(Target.Yaw, -90.f, 90.f);
	FRotator TargetRot = FRotator(Target.Pitch, ClampTargetYaw, Target.Roll);
	// if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
	if( DeltaTime == 0.f || Current == TargetRot )
	{
		return Current;
	}

	// If no interp speed, jump to target value
	if( InterpSpeed <= 0.f )
	{
		return Target;
	}

	const float DeltaInterpSpeed = InterpSpeed * DeltaTime;

	const FRotator Delta = (TargetRot - Current).GetNormalized();
	
	// If steps are too small, just return Target and assume we have reached our destination.
	if (Delta.IsNearlyZero())
	{
		return TargetRot;
	}

	// Delta Move, Clamp so we do not over shoot.
	const FRotator DeltaMove = Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f);
	FRotator Result = Current + DeltaMove;
	if(Result.Yaw > 90.f || Result.Yaw < -90.f)
	{
		Result = Current - DeltaMove;
	}

	Result.Normalize();
	double ClampYaw = FMath::Clamp(Result.Yaw, -90.0, 90.0);
	Result.Yaw = ClampYaw;
	
	return Result;
}

void ABasicCharacter::UpdateAim()
{
	// TODO: Fix Aim Offset
	FRotator ControlRot = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());
	FRotator CurrentRot = FRotator(AimOffset.X, AimOffset.Y, 0.f);
	
	FRotator ResultRot = RInterpTo(CurrentRot, ControlRot, GetWorld()->GetDeltaSeconds(), 10.f);
	FVector Result = FVector(ResultRot.Pitch, ResultRot.Yaw, 0.f);
		
	if(AimOffset != Result)
	{
		AimOffset = Result;
		Server_UpdateAim(AimOffset);
	}
}

void ABasicCharacter::Server_UpdateAim_Implementation(const FVector& InAimOffset)
{
	AimOffset = InAimOffset;
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

bool ABasicCharacter::IsDead()
{
	return Health->IsDead();
}

void ABasicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent) return;

	//Looking
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasicCharacter::Look);
}

