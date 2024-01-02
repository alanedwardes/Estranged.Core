// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EstVolumeSave.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API UEstVolumeSave : public USaveGame
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume)
    float MasterVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume)
	float EffectsVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume)
	float VoiceVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume)
	float MusicVolume;
};
