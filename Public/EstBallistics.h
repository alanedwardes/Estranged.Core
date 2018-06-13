#pragma once

#include "Engine/DataAsset.h"
#include "EstImpactEffect.h"
#include "EstBallistics.generated.h"

/**
Ballistics data for a projectile.
*/
UCLASS()
class ESTCORE_API UEstBallistics : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The projectile's damage characteristics. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Features)
	float Damage;

	/** The projectile's impact properties in relation to physical materials. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Features)
	TArray<struct FEstImpactEffect> ImpactEffects;

	/** Default sound effects, particles and sounds commenced upon impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Features)
	struct FEstImpactEffect DefaultImpactEffect;
};
