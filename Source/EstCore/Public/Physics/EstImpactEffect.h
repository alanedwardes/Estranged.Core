#pragma once

#include "EstImpactEffect.generated.h"

/** Describes the decals, sounds and particle systems emitted when a projectile impacts an object.  */
USTRUCT(BlueprintType)
struct ESTCORE_API FEstImpactEffect
{
	GENERATED_BODY()

	FEstImpactEffect() : Sound(nullptr), ScrapeSound(nullptr), ParticleSystem(nullptr), ParticleSystemDebris(nullptr) {}

	static const FEstImpactEffect None;

	FORCEINLINE bool operator==(const FEstImpactEffect& Effect) const
	{
		return ParticleSystem == Effect.ParticleSystem &&
			   ParticleSystemDebris == Effect.ParticleSystemDebris &&
			   Sound == Effect.Sound;
	}

	FORCEINLINE bool operator!=(const FEstImpactEffect& Effect) const
	{
		return ParticleSystem != Effect.ParticleSystem ||
			   ParticleSystemDebris != Effect.ParticleSystemDebris ||
			   Sound != Effect.Sound;
	}

	/** The sound played on impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	class USoundBase* Sound;

	/** The sound played on scrape. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	class USoundBase* ScrapeSound;

	/** The particle system started upon impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	class UParticleSystem* ParticleSystem;

	/** The particle system started upon impact, that isn't attached to the actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	class UParticleSystem* ParticleSystemDebris;
};