// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Interfaces/EstSaveRestore.h"
#include "EstGameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "EstAudioComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Audio, Common), hidecategories = (Object, ActorComponent, Physics, Rendering, Mobility, LOD), ShowCategories = Trigger, meta = (BlueprintSpawnableComponent))
class ESTCORE_API UEstAudioComponent : public UAudioComponent, public IEstSaveRestore
{
	GENERATED_BODY()
	
public:
	// Begin IEstSaveRestore
	void OnPreRestore_Implementation() override {};

	void OnPostRestore_Implementation() override
	{
		SetSound(SAVE_Sound);

		if (SAVE_bIsPlaying)
		{
			Play(SAVE_Position);
		}
	};

	void OnPreSave_Implementation() override
	{
		SAVE_Position = GetPlayPosition();
		SAVE_bIsPlaying = IsPlaying();
		SAVE_Sound = Sound;
	};

	void OnPostSave_Implementation() override {};
	// End IEstSaveRestore

	// Begin SaveId
	virtual FGuid GetSaveId_Implementation() override { return FGuid(); };
	// End SaveId

	virtual void Play(float StartTime = 0.f) override
	{
		Super::Play(StartTime);
		SoundGameStartTime = GetWorld()->TimeSeconds - StartTime;
	};

	UFUNCTION(BlueprintPure, Category = Sound)
	virtual float GetPlayPosition()
	{
		if (!IsPlaying())
		{
			return 0.f;
		}

		return GetWorld()->TimeSeconds - SoundGameStartTime;
	}

private:
	UPROPERTY(SaveGame)
	bool SAVE_bIsPlaying;

	UPROPERTY(SaveGame)
	float SAVE_Position;

	UPROPERTY(SaveGame)
	class USoundBase* SAVE_Sound;

	UPROPERTY()
	float SoundGameStartTime;
};
