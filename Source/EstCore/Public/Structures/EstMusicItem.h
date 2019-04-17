#pragma once

#include "EstMusicItem.generated.h"

/** Describes the decals, sounds and particle systems emitted when a projectile impacts an object.  */
USTRUCT(BlueprintType)
struct ESTCORE_API FEstMusicItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bIsLooping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	class USoundBase* Music;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float FadeOutTime;
};