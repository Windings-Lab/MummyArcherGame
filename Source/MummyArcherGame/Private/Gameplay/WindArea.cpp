#include "Gameplay/WindArea.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Gameplay/Interfaces/AffectedByWind.h"
#include "GameRules/MummyGameState.h"

AWindArea::AWindArea()
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetMobility(EComponentMobility::Type::Movable);
	SetRootComponent(ArrowComponent);
	
	BoxArea = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxArea"));
	BoxArea->SetupAttachment(RootComponent);
}

FVector AWindArea::GetDirection() const
{
	return ArrowComponent->GetForwardVector();
}

FVector AWindArea::GetAcceleration() const
{
	return GetDirection() * WindSpeed;
}

void AWindArea::BeginPlay()
{
	Super::BeginPlay();

	if(bGlobal)
	{

		BoxArea->OnComponentBeginOverlap.RemoveAll(this);
		BoxArea->OnComponentEndOverlap.RemoveAll(this);
		
		AMummyGameState* GameState = GetWorld()->GetGameState<AMummyGameState>();
		if(GameState)
		{
			GameState->SetWindDirection(GetDirection());
			GameState->SetWindSpeed(WindSpeed);
		}
	}
	if(!bGlobal)
	{
		BoxArea->OnComponentBeginOverlap.AddUniqueDynamic(this, &AWindArea::OnBeginOverlap);
		BoxArea->OnComponentEndOverlap.AddUniqueDynamic(this, &AWindArea::OnEndOverlap);
	}
}

void AWindArea::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(bGlobal)
	{
		BoxArea->OnComponentBeginOverlap.RemoveAll(this);
		BoxArea->OnComponentEndOverlap.RemoveAll(this);
		
		AMummyGameState* GameState = GetWorld()->GetGameState<AMummyGameState>();
		if(GameState)
		{
			GameState->SetWindDirection(GetDirection());
			GameState->SetWindSpeed(WindSpeed);
		}
	}
	if(!bGlobal)
	{
		BoxArea->OnComponentBeginOverlap.AddUniqueDynamic(this, &AWindArea::OnBeginOverlap);
		BoxArea->OnComponentEndOverlap.AddUniqueDynamic(this, &AWindArea::OnEndOverlap);
	}
}

void AWindArea::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if(bGlobal)
	{
		BoxArea->OnComponentBeginOverlap.RemoveAll(this);
		BoxArea->OnComponentEndOverlap.RemoveAll(this);
		
		AMummyGameState* GameState = GetWorld()->GetGameState<AMummyGameState>();
		if(GameState)
		{
			GameState->SetWindDirection(GetDirection());
			GameState->SetWindSpeed(WindSpeed);
		}
	}
	if(!bGlobal)
	{
		BoxArea->OnComponentBeginOverlap.AddUniqueDynamic(this, &AWindArea::OnBeginOverlap);
		BoxArea->OnComponentEndOverlap.AddUniqueDynamic(this, &AWindArea::OnEndOverlap);
	}
}

void AWindArea::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(bGlobal) return;
	
	if(IAffectedByWind* AffectedByWind = Cast<IAffectedByWind>(OtherActor))
	{
		AffectedByWind->SetWindModificator(GetAcceleration());
	}
}

void AWindArea::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(bGlobal) return;
	
	if(IAffectedByWind* AffectedByWind = Cast<IAffectedByWind>(OtherActor))
	{
		AffectedByWind->SetWindModificator(-GetAcceleration());
	}
}

