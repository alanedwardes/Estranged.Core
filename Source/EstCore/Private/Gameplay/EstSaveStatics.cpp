// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstSaveStatics.h"
#include "Gameplay/EstGameplayStatics.h"
#include "EstCore.h"
#include "Gameplay/EstPlayerController.h"
#include "Misc/UObjectToken.h"
#include "Logging/MessageLog.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"
#include "Gameplay/EstPlayer.h"
#include "UI/EstHUDWidget.h"
#include "Runtime/Core/Public/Serialization/MemoryReader.h"
#include "Runtime/CoreUObject/Public/Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Runtime/Engine/Classes/Engine/LevelStreaming.h"
#include "Runtime/Engine/Classes/Engine/Scene.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "LevelSequencePlayer.h"
#include "Saves/EstCheckpointSave.h"

bool UEstSaveStatics::IsActorValidForSaving(AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	if (!Actor->Implements<UEstSaveRestore>())
	{
		return false;
	}

	if (Actor->IsChildActor())
	{
		return false;
	}

	if (!IEstSaveRestore::Execute_GetSaveId(Actor).IsValid())
	{
		UE_LOG(LogEstGeneral, Warning, TEXT("Class %s does not implement GetSaveId(). This actor will be skipped in save games"), *Actor->GetClass()->GetName());
#if WITH_EDITOR
		FMessageLog("PIE").Error()
			->AddToken(FTextToken::Create(FText::FromString("Class")))
			->AddToken(FUObjectToken::Create(Actor->GetClass()))
			->AddToken(FTextToken::Create(FText::FromString("does not implement GetSaveId(). This actor will be skipped in save games")));
#endif
		return false;
	}

	if (IEstSaveRestore::Execute_GetSaveId(Actor) != IEstSaveRestore::Execute_GetSaveId(Actor))
	{
		UE_LOG(LogEstGeneral, Warning, TEXT("Class %s does have a deterministic implementation of GetSaveId(). This actor will be skipped in save games"), *Actor->GetClass()->GetName());
#if WITH_EDITOR
		FMessageLog("PIE").Error()
			->AddToken(FTextToken::Create(FText::FromString("Class")))
			->AddToken(FUObjectToken::Create(Actor->GetClass()))
			->AddToken(FTextToken::Create(FText::FromString("does have a deterministic implementation of GetSaveId(). This actor will be skipped in save games")));
#endif
		return false;
	}

	return true;
}

FEstWorldState UEstSaveStatics::SerializeWorld(UObject* WorldContextObject)
{
	FEstWorldState WorldState;

	UWorld* World = WorldContextObject->GetWorld();

	SerializeLevel(World->PersistentLevel, WorldState.PersistentLevelState);
	WorldState.PersistentLevelState.LevelName = World->PersistentLevel->GetOuter()->GetFName();

	for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
	{
		ULevel* LoadedLevel = StreamingLevel->GetLoadedLevel();
		if (!LoadedLevel)
		{
			continue;
		}

		FEstLevelState LevelState;
		SerializeLevel(LoadedLevel, LevelState);
		LevelState.LevelName = StreamingLevel->GetWorldAssetPackageFName();
		WorldState.StreamingLevelStates.Add(LevelState);
	}
	return WorldState;
}

void UEstSaveStatics::SerializeLevel(ULevel* Level, FEstLevelState &LevelState)
{
	for (AActor* Actor : TSet<AActor*>(Level->Actors))
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		if (Actor->ActorHasTag(TAG_NOSAVERESTORE))
		{
			continue;
		}

		if (Actor->GetClass()->IsChildOf(ALevelSequenceActor::StaticClass()))
		{
			ALevelSequenceActor* LevelSequenceActor = Cast<ALevelSequenceActor>(Actor);
			if (LevelSequenceActor->SequencePlayer == nullptr)
			{
				continue;
			}

			LevelState.SequenceStates.Add(SerializeSequence(LevelSequenceActor));
		}

		if (IsActorValidForSaving(Actor))
		{
			FEstActorState ActorState;
			SerializeActor(Actor, ActorState);
			LevelState.ActorStates.Add(ActorState);
		}
	}
}

void UEstSaveStatics::RestoreWorld(UObject* WorldContextObject, FEstWorldState WorldState)
{
	UWorld* World = WorldContextObject->GetWorld();

	TArray<UObject*> RestoredObjects;

	RestoreLevel(World->PersistentLevel, WorldState.PersistentLevelState, RestoredObjects);

	for (FEstLevelState StreamingLevelState : WorldState.StreamingLevelStates)
	{
		const ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(WorldContextObject, StreamingLevelState.LevelName);
		const ULevel* FoundLevel = StreamingLevel->GetLoadedLevel();
		
		// FoundLevel could be null if the level was being unloaded at save time
		if (FoundLevel != nullptr)
		{
			RestoreLevel(FoundLevel, StreamingLevelState, RestoredObjects);
		}
	}

	for (UObject* RestoredObject : RestoredObjects)
	{
		IEstSaveRestore::Execute_OnPostRestore(RestoredObject);
	}
}

void UEstSaveStatics::RestoreLevel(const ULevel* Level, FEstLevelState LevelState, TArray<UObject*> &RestoredObjects)
{
	// Restore all actors
	for (const FEstActorState &ActorState : LevelState.ActorStates)
	{
		AActor* FoundActor = UEstGameplayStatics::FindActorBySaveIdInWorld(Level->GetWorld(), ActorState.SaveId);
		if (FoundActor == nullptr)
		{
			// We likely have an actor which was moved
			UE_LOG(LogEstGeneral, Warning, TEXT("Unable to find level actor %s by save ID %s, spawning"), *ActorState.ActorClass->GetName(), *ActorState.SaveId.ToString());
			FoundActor = SpawnMovedActor(Level->GetWorld(), ActorState);
		}

		RestoreActor(FoundActor, ActorState);

		// Restore all components
		for (FEstComponentState ComponentState : ActorState.ComponentStates)
		{
			RestoredObjects.Add(RestoreComponent(FoundActor, ComponentState));
		}

		RestoredObjects.Add(FoundActor);
	}

	// Restore all level sequences
	for (const FEstSequenceState &SequenceState : LevelState.SequenceStates)
	{
		RestoreSequence(Level->GetWorld(), SequenceState);
	}
}

FEstSequenceState UEstSaveStatics::SerializeSequence(ALevelSequenceActor* LevelSequenceActor)
{
	FEstSequenceState SequenceState;
	SequenceState.ActorName = LevelSequenceActor->GetFName();
	SequenceState.FrameNumber = LevelSequenceActor->SequencePlayer->GetCurrentTime().Time.FrameNumber.Value;
	SequenceState.PlayRate = LevelSequenceActor->SequencePlayer->GetPlayRate();
	SequenceState.bIsPlaying = LevelSequenceActor->SequencePlayer->IsPlaying();
	return SequenceState;
}

void UEstSaveStatics::SerializeActor(AActor* Actor, FEstActorState& ActorState)
{
	IEstSaveRestore::Execute_OnPreSave(Actor);

	ActorState.SaveId = IEstSaveRestore::Execute_GetSaveId(Actor);
	ActorState.ActorName = Actor->GetFName();
	ActorState.ActorClass = Actor->GetClass();
	ActorState.ActorTransform = Actor->GetActorTransform();
	ActorState.ActorTags = Actor->Tags;
	SerializeLowLevel(Actor, ActorState.ActorData);

	for (UActorComponent* Component : Actor->GetComponents())
	{
		if (!Component->Implements<UEstSaveRestore>())
		{
			continue;
		}

		ActorState.ComponentStates.Add(SerializeComponent(Component));
	}

	IEstSaveRestore::Execute_OnPostSave(Actor);
}

FEstComponentState UEstSaveStatics::SerializeComponent(UActorComponent* Component)
{
	IEstSaveRestore::Execute_OnPreSave(Component);

	FEstComponentState ComponentState;
	ComponentState.ComponentName = Component->GetFName();
	ComponentState.ComponentClass = Component->GetClass();
	SerializeLowLevel(Component, ComponentState.ComponentData);
	const USceneComponent* SceneComponent = Cast<USceneComponent>(Component);
	if (SceneComponent != nullptr)
	{
		ComponentState.ComponentTransform = SceneComponent->GetRelativeTransform();
	}

	IEstSaveRestore::Execute_OnPostSave(Component);

	return ComponentState;
}

UActorComponent* UEstSaveStatics::RestoreComponent(AActor* Parent, const FEstComponentState &ComponentState)
{
	UActorComponent* FoundComponent = nullptr;

	TArray<UActorComponent*> ActorComponents;
	Parent->GetComponents(ActorComponents);
	for (UActorComponent* Component : ActorComponents)
	{
		if (Component->GetFName() == ComponentState.ComponentName)
		{
			FoundComponent = Component;
			break;
		}
	}

	check(FoundComponent);

	IEstSaveRestore::Execute_OnPreRestore(FoundComponent);

	USceneComponent* SceneComponent = Cast<USceneComponent>(FoundComponent);
	if (SceneComponent && SceneComponent->Mobility == EComponentMobility::Movable)
	{
		SceneComponent->SetRelativeTransform(ComponentState.ComponentTransform);
	}

	RestoreLowLevel(FoundComponent, ComponentState.ComponentData);

	return FoundComponent;
}

AActor* UEstSaveStatics::SpawnMovedActor(UWorld* World, const FEstActorState &ActorState)
{
	FActorSpawnParameters Parameters;
	Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Parameters.bNoFail = true;

	AActor* SpawnedActor = World->SpawnActor<AActor>(ActorState.ActorClass, ActorState.ActorTransform, Parameters);

	IEstSaveRestore::Execute_OnPreRestore(SpawnedActor);

	SpawnedActor->Tags = ActorState.ActorTags;

	RestoreLowLevel(SpawnedActor, ActorState.ActorData);

	return SpawnedActor;
}

void UEstSaveStatics::RestoreActor(AActor* Actor, const FEstActorState &ActorState)
{
	IEstSaveRestore::Execute_OnPreRestore(Actor);

	Actor->Tags = ActorState.ActorTags;

	const bool bShouldIgnoreTransform = EnumHasAnyFlags(IEstSaveRestore::Execute_GetSaveFlags(Actor), EEstSaveFlags::IgnoreTransform);
	if (!bShouldIgnoreTransform && Actor->GetRootComponent() && Actor->GetRootComponent()->Mobility == EComponentMobility::Movable)
	{
		Actor->SetActorTransform(ActorState.ActorTransform);
	}

	RestoreLowLevel(Actor, ActorState.ActorData);
}

ALevelSequenceActor* UEstSaveStatics::RestoreSequence(UWorld* World, const FEstSequenceState SequenceState)
{
	for (TActorIterator<ALevelSequenceActor> LevelSequenceActor(World); LevelSequenceActor; ++LevelSequenceActor)
	{
		if (!IsValid(*LevelSequenceActor))
		{
			continue;
		}

		if (LevelSequenceActor->GetFName() == SequenceState.ActorName)
		{
			// There have been crash reports from this area of the code, do some sanity checks
			check(LevelSequenceActor->SequencePlayer != nullptr);

			const bool bIsAtStart = SequenceState.FrameNumber == 0;
			const bool bIsAtEnd = SequenceState.FrameNumber >= LevelSequenceActor->SequencePlayer->GetEndTime().Time.FrameNumber.Value;

			LevelSequenceActor->SequencePlayer->SetPlayRate(SequenceState.PlayRate);

			// Don't explicitly set the position if at start
			// this can cause frames to get dropped (and events on frame 0 skipped)
			if (!bIsAtStart)
			{
				FMovieSceneSequencePlaybackParams PlaybackParams;
				PlaybackParams.Frame = FFrameTime(SequenceState.FrameNumber);
				LevelSequenceActor->SequencePlayer->SetPlaybackPosition(PlaybackParams);
			}

			// Start playing if at end of sequence too, this
			// causes any events on the final frame to be fired
			if (SequenceState.bIsPlaying || bIsAtEnd)
			{
				LevelSequenceActor->SequencePlayer->Play();
			}

			return *LevelSequenceActor;
		}
	}

	return nullptr;
}

void UEstSaveStatics::RestoreLowLevel(UObject* Object, TArray<uint8> Bytes)
{
	FMemoryReader MemoryReader(Bytes, true);
	FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
	Object->Serialize(Ar);
}

void UEstSaveStatics::SerializeLowLevel(UObject* Object, TArray<uint8>& InBytes)
{
	FMemoryWriter MemoryWriter(InBytes, true);
	FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, true);
	Ar.ArIsSaveGame = true;
	Ar.ArNoDelta = true;
	Object->Serialize(Ar);
}

UEstCheckpointSave* UEstSaveStatics::LoadCheckpoints()
{
	UEstCheckpointSave* Save = Cast<UEstCheckpointSave>(UGameplayStatics::LoadGameFromSlot(SAVE_SLOT_CHECKPOINTS, 0));
	if (Save == nullptr)
	{
		Save = NewObject<UEstCheckpointSave>();
	}

	return Save;
}

void UEstSaveStatics::SaveCheckpoints(UEstCheckpointSave* Checkpoints)
{
	UGameplayStatics::SaveGameToSlot(Checkpoints, SAVE_SLOT_CHECKPOINTS, 0);
}

void UEstSaveStatics::AddCheckpoint(UObject* WorldContextObject, FEstCheckpoint NewCheckpoint)
{
	UEstCheckpointSave* Save = LoadCheckpoints();

	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(WorldContextObject);

	// Replace all other checkpoints for this level
	Save->Checkpoints = Save->Checkpoints.FilterByPredicate([&CurrentLevelName](const FEstCheckpoint& Checkpoint)
	{
		return !CurrentLevelName.Equals(Checkpoint.Level);
	});

	NewCheckpoint.Level = CurrentLevelName;
	NewCheckpoint.CreatedOn = FDateTime::UtcNow();
	Save->Checkpoints.Add(NewCheckpoint);
	SaveCheckpoints(Save);
}

FEstCheckpoint UEstSaveStatics::GetLastCheckpoint(UObject* WorldContextObject, bool bCurrentLevelOnly, bool& bIsValid)
{
	UEstCheckpointSave* Save = LoadCheckpoints();

	TArray<FEstCheckpoint> Checkpoints = TArray(Save->Checkpoints);

	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(WorldContextObject);

	if (bCurrentLevelOnly)
	{
		Checkpoints = Checkpoints.FilterByPredicate([&CurrentLevelName](const FEstCheckpoint& Checkpoint)
		{
			return CurrentLevelName.Equals(Checkpoint.Level);
		});
	}

	Checkpoints.Sort([](const FEstCheckpoint& LHS, const FEstCheckpoint& RHS)
	{
		return LHS.CreatedOn > RHS.CreatedOn;
	});

	if (Checkpoints.Num() > 0)
	{
		bIsValid = true;
		return Checkpoints[0];
	}

	bIsValid = false;
	return FEstCheckpoint();
}

void UEstSaveStatics::OpenCheckpoint(UObject* WorldContextObject, FEstCheckpoint NewCheckpoint)
{
	FString LevelString = FString::Printf(TEXT("%s#%s"), *NewCheckpoint.Level, *NewCheckpoint.PlayerStartTag.ToString());
	UGameplayStatics::OpenLevel(WorldContextObject, FName(*LevelString));
}

bool UEstSaveStatics::PersistSaveRaw(const TArray<uint8>& SrcData, const FString & SlotName, const int32 UserIndex)
{
	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	if (SaveSystem && (SlotName.Len() > 0))
	{
		return SaveSystem->SaveGame(false, *SlotName, UserIndex, SrcData);
	}

	return false;
}

bool UEstSaveStatics::LoadSaveRaw(TArray<uint8>& DstData, const FString & SlotName, const int32 UserIndex)
{
	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	if (SaveSystem && (SlotName.Len() > 0))
	{
		return SaveSystem->LoadGame(false, *SlotName, UserIndex, DstData);
	}

	return false;
}
