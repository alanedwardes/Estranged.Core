// Estranged is a trade mark of Alan Edwardes.

#include "Volumes/EstFootstepVolume.h"
#include "EstCore.h"
#include "Components/BrushComponent.h"
#include "Gameplay/EstFootstepComponent.h"

AEstFootstepVolume::AEstFootstepVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetBrushComponent()->SetCollisionProfileName(PROFILE_TRIGGER);
}

void AEstFootstepVolume::NotifyActorBeginOverlap(AActor *OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	UEstFootstepComponent* FootstepComponent = OtherActor->FindComponentByClass<UEstFootstepComponent>();
	if (FootstepComponent == nullptr)
	{
		return;
	}

	FootstepComponent->FootstepMaterialOverride = FootstepMaterialOverride;
}

void AEstFootstepVolume::NotifyActorEndOverlap(AActor *OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	UEstFootstepComponent* FootstepComponent = OtherActor->FindComponentByClass<UEstFootstepComponent>();
	if (FootstepComponent == nullptr)
	{
		return;
	}

	if (FootstepComponent->FootstepMaterialOverride != FootstepMaterialOverride)
	{
		// Something else must have set this, let it unset.
		return;
	}

	FootstepComponent->FootstepMaterialOverride = nullptr;
}
