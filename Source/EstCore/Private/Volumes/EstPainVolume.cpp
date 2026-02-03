// Estranged is a trade mark of Alan Edwardes.

#include "Volumes/EstPainVolume.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "Components/BrushComponent.h"
#include "EstConstants.h"
#include "TimerManager.h"

AEstPainVolume::AEstPainVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	GetBrushComponent()->SetCollisionProfileName(PROFILE_TRIGGER);

	bPainCausing = true;
	DamagePerSec = 10.f;
	PainInterval = 1.f;
	bEntryPain = true;
	DamageType = UDamageType::StaticClass();
}

void AEstPainVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsValid(OtherActor) && bPainCausing)
	{
		ActorsToSuffer.Add(OtherActor);

		if (bEntryPain)
		{
			CausePainTo(OtherActor);
		}

		if (!GetWorldTimerManager().IsTimerActive(PainTimerHandle))
		{
			GetWorldTimerManager().SetTimer(PainTimerHandle, this, &AEstPainVolume::PainTimer, PainInterval, true);
		}
	}
}

void AEstPainVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (IsValid(OtherActor))
	{
		ActorsToSuffer.Remove(OtherActor);
	}

	if (ActorsToSuffer.Num() == 0)
	{
		GetWorldTimerManager().ClearTimer(PainTimerHandle);
	}
}

void AEstPainVolume::PainTimer()
{
	if (!bPainCausing)
	{
		return;
	}

	for (auto It = ActorsToSuffer.CreateIterator(); It; ++It)
	{
		AActor* Actor = It->Get();
		if (IsValid(Actor))
		{
			CausePainTo(Actor);
		}
		else
		{
			It.RemoveCurrent();
		}
	}

	if (ActorsToSuffer.Num() == 0)
	{
		GetWorldTimerManager().ClearTimer(PainTimerHandle);
	}
}

void AEstPainVolume::CausePainTo(AActor* Other)
{
	if (!IsValid(Other))
	{
		return;
	}

	TSubclassOf<UDamageType> DmgTypeClass = DamageType ? *DamageType : UDamageType::StaticClass();
	Other->TakeDamage(DamagePerSec * PainInterval, FDamageEvent(DmgTypeClass), nullptr, this);
}
