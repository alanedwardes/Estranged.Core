#pragma once

#include "GameFramework/Actor.h"
#include "SoundDefinitions.h"
#include "EstSoundscapeParameters.generated.h"

USTRUCT(BlueprintType)
struct FEstSoundscapeParameters
{
	GENERATED_BODY()

	FEstSoundscapeParameters()
	{
		Sound = nullptr;
		Reverb = nullptr;
	}

	UPROPERTY(EditAnywhere, SaveGame, BlueprintReadWrite, Category = "Soundscape")
	USoundBase* Sound;

	UPROPERTY(EditAnywhere, SaveGame, BlueprintReadWrite, Category = "Soundscape")
	UReverbEffect* Reverb;

	friend FArchive& operator<<(FArchive& Archive, FEstSoundscapeParameters& SoundscapeParameters)
	{
		Archive << SoundscapeParameters.Sound;
		Archive << SoundscapeParameters.Reverb;
		return Archive;
	}
};