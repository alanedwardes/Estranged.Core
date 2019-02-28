// Estranged is a trade mark of Alan Edwardes.

#include "EstFootstepVolume.h"
#include "EstCore.h"
#include "EstCharacterMovementComponent.h"

AEstFootstepVolume::AEstFootstepVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetBrushComponent()->SetCollisionProfileName(PROFILE_TRIGGER);
}

void AEstFootstepVolume::NotifyActorBeginOverlap(AActor *OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	UEstCharacterMovementComponent* MovementComponent = OtherActor->FindComponentByClass<UEstCharacterMovementComponent>();
	if (MovementComponent == nullptr)
	{
		return;
	}

	MovementComponent->FootstepMaterialOverride = FootstepMaterialOverride;
}

void AEstFootstepVolume::NotifyActorEndOverlap(AActor *OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	UEstCharacterMovementComponent* MovementComponent = OtherActor->FindComponentByClass<UEstCharacterMovementComponent>();
	if (MovementComponent == nullptr)
	{
		return;
	}

	if (MovementComponent->FootstepMaterialOverride != FootstepMaterialOverride)
	{
		// Something else must have set this, let it unset.
		return;
	}

	MovementComponent->FootstepMaterialOverride = nullptr;
}
