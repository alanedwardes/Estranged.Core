// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Saves/EstGameState.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Interfaces/EstSaveRestore.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "EstSaveStatics.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API UEstSaveStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = Saving)
	static class UEstGameSettingsSave* LoadGameSettings();

	UFUNCTION(BlueprintCallable, Category = Saving)
	static void SaveGameSettings(UEstGameSettingsSave* GameSettings);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static class UEstCheckpointSave* LoadCheckpoints();

	UFUNCTION(BlueprintCallable, Category = Saving)
	static void SaveCheckpoints(UEstCheckpointSave* Checkpoints);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = Saving)
	static void AddCheckpoint(UObject* WorldContextObject, FEstCheckpoint NewCheckpoint);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = Saving)
	static FEstCheckpoint GetLastCheckpoint(UObject* WorldContextObject, bool bCurrentLevelOnly, bool& bIsValid);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = Saving)
	static void OpenCheckpoint(UObject* WorldContextObject, FEstCheckpoint NewCheckpoint);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static bool PersistSaveRaw(const TArray<uint8> &SrcData, const FString& SlotName, const int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static bool LoadSaveRaw(TArray<uint8> &DstData, const FString& SlotName, const int32 UserIndex);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = Saving)
	static FEstWorldState SerializeWorld(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static void SerializeLevel(ULevel* Level, FEstLevelState &LevelState);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = Saving)
	static void RestoreWorld(UObject* WorldContextObject, FEstWorldState WorldState);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static void RestoreLevel(const ULevel* Level, FEstLevelState LevelState, TArray<UObject*> &RestoredObjects);
	
	UFUNCTION(BlueprintCallable, Category = Saving)
	static FEstSequenceState SerializeSequence(ALevelSequenceActor* LevelSequenceActor);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static void SerializeActor(AActor* Actor, FEstActorState& ActorState);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static FEstComponentState SerializeComponent(UActorComponent* Component);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static UActorComponent* RestoreComponent(AActor* Parent, const FEstComponentState &ComponentState);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static AActor* SpawnMovedActor(UWorld *World, const FEstActorState &ActorState);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static void RestoreActor(AActor* Actor, const FEstActorState &ActorState);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static ALevelSequenceActor* RestoreSequence(UWorld* World, const FEstSequenceState SequenceState);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static void RestoreLowLevel(UObject* Object, TArray<uint8> Bytes);

	UFUNCTION(BlueprintCallable, Category = Saving)
	static void SerializeLowLevel(UObject* Object, TArray<uint8>& InBytes);

	static bool IsActorValidForSaving(AActor* Actor);
};
