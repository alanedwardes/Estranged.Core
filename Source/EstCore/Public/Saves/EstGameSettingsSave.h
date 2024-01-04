// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EstGameSettingsSave.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API UEstGameSettingsSave : public USaveGame
{
	GENERATED_BODY()
public:
	UEstGameSettingsSave()
	{
		MasterVolume = 1.f;
		EffectsVolume = 1.f;
		VoiceVolume = 1.f;
		MusicVolume = 1.f;
		FieldOfView = 110.f;
	}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume)
    float MasterVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume)
	float EffectsVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume)
	float VoiceVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume)
	float MusicVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float FieldOfView;
};
