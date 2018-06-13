#pragma once

#include "Engine/DataAsset.h"
#include "EstImpactManifest.generated.h"

/** Links physical materials to footstep sounds. */
UCLASS(BlueprintType)
class ESTCORE_API UEstImpactManifest : public UDataAsset
{
	GENERATED_BODY()

public:
	UEstImpactManifest(const class FObjectInitializer& PCIP) : Super(PCIP)
	{
	}

	/** The sound effects, particles and sounds commenced upon impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Footsteps)
	TArray<struct FEstImpactEffect> ImpactEffects;
};