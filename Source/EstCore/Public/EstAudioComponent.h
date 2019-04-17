// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Interfaces/EstSaveRestore.h"
#include "EstGameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "EstAudioComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSoundPlayed);

/**
 * 
 */
UCLASS(ClassGroup = (Audio, Common), hidecategories = (Object, ActorComponent, Physics, Rendering, Mobility, LOD), ShowCategories = Trigger, meta = (BlueprintSpawnableComponent))
class ESTCORE_API UEstAudioComponent : public UAudioComponent, public IEstSaveRestore
{
	GENERATED_BODY()

	UEstAudioComponent(const FObjectInitializer& ObjectInitializer);
	
public:
	// Begin IEstSaveRestore
	void OnPreRestore_Implementation() override {};
	void OnPostRestore_Implementation() override;
	void OnPreSave_Implementation() override;
	void OnPostSave_Implementation() override {};
	// End IEstSaveRestore

	// Begin SaveId
	virtual FGuid GetSaveId_Implementation() override { return FGuid(); };
	// End SaveId

	UPROPERTY(BlueprintAssignable)
	FOnSoundPlayed OnSoundPlayed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame)
	float FadeOutTime;

	virtual void Play(float StartTime = 0.f);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction);

	UFUNCTION(BlueprintPure, Category = Sound)
	virtual float GetPlayPosition();

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
