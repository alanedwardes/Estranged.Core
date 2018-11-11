// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "EstSave.h"
#include "EstGameplaySave.h"
#include "EstLevelUnlockSave.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EstSaveRestore.h"
#include "EstGameState.h"
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
	template <typename TSaveType>
	static TSaveType* LoadSave()
	{
		TSaveType* DefaultSave = Cast<TSaveType>(UGameplayStatics::CreateSaveGameObject(TSaveType::StaticClass()));
		TSaveType* LoadedSave = Cast<TSaveType>(UGameplayStatics::LoadGameFromSlot(DefaultSave->GetSlotName(), 0));
		return LoadedSave == nullptr ? DefaultSave : LoadedSave;
	};

	UFUNCTION(BlueprintCallable, Category = Saving)
	static UEstSave* LoadSave(UClass* SaveClass)
	{
		UEstSave* DefaultSave = Cast<UEstSave>(UGameplayStatics::CreateSaveGameObject(SaveClass));
		UEstSave* LoadedSave = Cast<UEstSave>(UGameplayStatics::LoadGameFromSlot(DefaultSave->GetSlotName(), 0));
		return LoadedSave == nullptr ? DefaultSave : LoadedSave;
	};

	UFUNCTION(BlueprintCallable, Category = Saving)
	static bool PersistSave(UEstSave* SaveGame);
	
	UFUNCTION(BlueprintCallable, Category = Saving)
	static UEstGameplaySave* LoadGameplaySave() { return LoadSave<UEstGameplaySave>(); };

	UFUNCTION(BlueprintCallable, Category = Saving)
	static UEstLevelUnlockSave* LoadLevelUnlockSave() { return LoadSave<UEstLevelUnlockSave>(); };

	UFUNCTION(BlueprintCallable, Category = Saving)
	static void ApplyGameplaySave(UEstGameplaySave* GameplaySave, class APlayerController* Controller);

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
