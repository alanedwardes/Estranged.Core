#pragma once

#include "EstMusic.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FEstMusic
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		USoundBase* Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		bool bShouldFadeCurrent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
		float Position;
};