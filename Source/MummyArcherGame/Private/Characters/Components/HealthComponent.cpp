// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Components/HealthComponent.h"

#include "Characters/MummyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GameHUDWidget.h"
#include "UI/HealthWidget.h"
#include "UI/MummyHUD.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = false;
	Health = DefaultHealth;
}

void UHealthComponent::Hit(int32 Parameter)
{
	if (Parameter > 0)
	{
		Parameter *= -1;
	}
	ChangeHealth(Parameter);
}

void UHealthComponent::Heal(int32 Parameter)
{
	if (Parameter < 0)
	{
		Parameter *= -1;
	}
	if (Health + Parameter >= 0)
	{
		ChangeHealth(Parameter);
	}
	else
	{
		ChangeHealth(-Health);
	}
}

void UHealthComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UHealthComponent::ChangeHealth(int32 Parameter)
{
	Health += Parameter;
	
	if (Cast<ABasicCharacter>(GetOwner())->GetController() && !Cast<ABasicCharacter>(GetOwner())->GetController()->HasAuthority())
	{
		if (!GameHUDWidget)
		{
			GameHUDWidget = Cast<UGameHUDWidget>(Cast<AMummyHUD>(Cast<APlayerController>(Cast<ABasicCharacter>(GetOwner())->GetController())->GetHUD())->GetMainWidget());
		}
		GameHUDWidget->GetHealthWidget()->UpdateHealth(static_cast<float>(Health) / static_cast<float>(DefaultHealth));
	}	
}
