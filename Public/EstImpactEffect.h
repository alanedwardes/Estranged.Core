#pragma once

#include "EstImpactEffect.generated.h"

/** Describes the decals, sounds and particle systems emitted when a projectile impacts an object.  */
USTRUCT(BlueprintType)
struct ESTCORE_API FEstImpactEffect
{
	GENERATED_USTRUCT_BODY()

	FEstImpactEffect()
	{
		PhysicalMaterial = nullptr;
		ParticleSystem = nullptr;
		Decal = nullptr;
		Sound = nullptr;
		DecalSize = FVector(5.f);
	}

	/** The physical material linked to the decal and sound. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImpactData)
	class UPhysicalMaterial* PhysicalMaterial;

	/** The decal placed upon impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Decal)
	class UMaterialInterface* Decal;

	/** The placed decal's size. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Decal)
	FVector DecalSize;

	/** The sound played on impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	class USoundBase* Sound;

	/** The particle system started upon impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	class UParticleSystem* ParticleSystem;
};
