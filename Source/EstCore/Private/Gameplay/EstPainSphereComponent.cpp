// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstPainSphereComponent.h"
#include "Volumes/EstWaterVolume.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "EstConstants.h"

void UEstPainSphereComponent::OnPainComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	if (bInverted)
	{
		ActorsToSuffer.Remove(OtherActor);
	}
	else
	{
		ActorsToSuffer.Add(OtherActor);
	}
}

void UEstPainSphereComponent::OnPainComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	if (bInverted)
	{
		ActorsToSuffer.Add(OtherActor);
	}
	else
	{
		ActorsToSuffer.Remove(OtherActor);
	}
}

UEstPainSphereComponent::UEstPainSphereComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	bInverted = false;
	DamagePerSec = 10.f;
	PainInterval = 1.f;
	DamageType = UDamageType::StaticClass();
	LastPainTime = 0.f;

	SetCollisionProfileName(PROFILE_TRIGGER);

	OnComponentBeginOverlap.AddDynamic(this, &UEstPainSphereComponent::OnPainComponentBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UEstPainSphereComponent::OnPainComponentEndOverlap);
}

void UEstPainSphereComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetWorld()->GetTimeSeconds() - LastPainTime < PainInterval)
	{
		return;
	}

	// Loop over actors to suffer
	// We need to handle the case where actors become invalid (destroyed)
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
	
	LastPainTime = GetWorld()->GetTimeSeconds();
}

void UEstPainSphereComponent::CausePainTo(AActor* Other)
{
	if (!IsValid(Other))
	{
		return;
	}

	TSubclassOf<UDamageType> DmgTypeClass = DamageType ? *DamageType : UDamageType::StaticClass();
	Other->TakeDamage(DamagePerSec * PainInterval, FDamageEvent(DmgTypeClass), nullptr, GetOwner());
}
