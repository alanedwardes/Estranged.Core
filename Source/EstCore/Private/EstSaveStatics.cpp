// Estranged is a trade mark of Alan Edwardes.

#include "EstSaveStatics.h"
#include "EstCore.h"
#include "EstPlayerController.h"
#include "Misc/UObjectToken.h"
#include "Logging/MessageLog.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"
#include "EstPlayer.h"
#include "EstHUDWidget.h"
#include "Runtime/Core/Public/Serialization/MemoryReader.h"
#include "Runtime/CoreUObject/Public/Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Runtime/Engine/Classes/Engine/LevelStreaming.h"
#include "Runtime/Engine/Classes/Engine/Scene.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Runtime/Engine/Public/EngineUtils.h"

void ApplyPostProcessingSettings(AEstPlayer* Player, UEstGameplaySave* GameplaySave)
{
	if (Player->Camera == nullptr)
	{
		EST_WARNING(TEXT("Attempting to apply gameplay settings with null player camera"));
		return;
	}

	Player->Camera->FieldOfView = GameplaySave->GetFieldOfView();

	Player->Camera->PostProcessSettings.bOverride_MotionBlurAmount = true;
	Player->Camera->PostProcessSettings.MotionBlurAmount = GameplaySave->GetDisableMotionBlur() ? 0.f : 1.f;
	Player->Camera->PostProcessSettings.bOverride_MotionBlurMax = true;
	Player->Camera->PostProcessSettings.MotionBlurMax = GameplaySave->GetDisableMotionBlur() ? 0.f : 1.f;

	Player->Camera->PostProcessSettings.bOverride_ColorGain = true;
	Player->Camera->PostProcessSettings.ColorGain = FVector4(GameplaySave->GetGamma(), GameplaySave->GetGamma(), GameplaySave->GetGamma(), GameplaySave->GetGamma());

	Player->Camera->PostProcessSettings.bOverride_AmbientOcclusionIntensity = true;
	Player->Camera->PostProcessSettings.AmbientOcclusionIntensity = GameplaySave->GetDisableAmbientOcclusion() ? 0.f : .5f;

	Player->UpdatePostProcessingTick(0.f);
}

void ApplyPlayerSettings(AEstPlayer* Player, UEstGameplaySave* GameplaySave)
{
	Player->RestingFieldOfView = GameplaySave->GetFieldOfView();
	Player->bDisableDepthOfField = GameplaySave->GetDisableDepthOfField();
	Player->bDisableVignette = GameplaySave->GetDisableVignette();
}

void ApplyPlayerControllerSettings(AEstPlayerController* PlayerController, UEstGameplaySave* GameplaySave)
{
	static IConsoleVariable* CVarForwardShading = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForwardShading"));
	const EAntiAliasingMethod AlternateAntiAliasingMethod = CVarForwardShading->GetInt() == 0 ? EAntiAliasingMethod::AAM_FXAA : EAntiAliasingMethod::AAM_MSAA;

	const EAntiAliasingMethod AntiAliasingType = GameplaySave->GetDisableTemporalAntiAliasing() ? AlternateAntiAliasingMethod : EAntiAliasingMethod::AAM_TemporalAA;
	PlayerController->ConsoleCommand(FString::Printf(TEXT("r.DefaultFeature.AntiAliasing %i"), (int)AntiAliasingType));
	PlayerController->UpdateCameraManager(0.f);
}

void ApplyHudWidgetSettings(UEstHUDWidget* HUDWidget, UEstGameplaySave* GameplaySave)
{
	if (HUDWidget == nullptr)
	{
		EST_WARNING(TEXT("Attempting to apply gameplay settings with null HUD"));
		return;
	}

	HUDWidget->bDisableHUD = GameplaySave->GetDisableHUD();
	HUDWidget->bDisableSubtitles = GameplaySave->GetDisableSubtitles();
	HUDWidget->bEnableStatsForNerds = GameplaySave->GetEnableStatsForNerds();
}

void UEstSaveStatics::ApplyGameplaySave(UEstGameplaySave* GameplaySave, APlayerController* Controller)
{
	if (GameplaySave == nullptr)
	{
		EST_WARNING(TEXT("Attempting to apply gameplay settings with null save"));
		return;
	}

	AEstPlayerController* PlayerController = Cast<AEstPlayerController>(Controller);
	if (PlayerController == nullptr)
	{
		EST_WARNING(TEXT("Attempting to apply gameplay settings with null player controller"));
		return;
	}

	AEstPlayer* Player = Cast<AEstPlayer>(PlayerController->GetPawn());
	if (Player == nullptr)
	{
		EST_WARNING(TEXT("Attempting to apply gameplay settings with null player or player camera"));
		return;
	}

	ApplyPostProcessingSettings(Player, GameplaySave);

	ApplyPlayerSettings(Player, GameplaySave);

	ApplyPlayerControllerSettings(PlayerController, GameplaySave);

	ApplyHudWidgetSettings(Player->HUDWidget, GameplaySave);
}

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

bool UEstSaveStatics::PersistSave(UEstSave* SaveGame)
{
	if (SaveGame == nullptr)
	{
		EST_WARNING(TEXT("UEstSaveStatics::PersistSave() called with null save game object!"));
		return false;
	}

	return UGameplayStatics::SaveGameToSlot(SaveGame, SaveGame->GetSlotName(), 0);
};

FEstWorldState UEstSaveStatics::SerializeWorld(UObject* WorldContextObject)
{
	FEstWorldState WorldState;

	UWorld* World = WorldContextObject->GetWorld();

	SerializeLevel(World->PersistentLevel, WorldState.PersistentLevelState);

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
		if (Actor == nullptr)
		{
			continue;
		}

		if (Actor->IsPendingKill())
		{
			continue;
		}

		if (Actor->GetClass()->IsChildOf(ALevelSequenceActor::StaticClass()))
		{
			ALevelSequenceActor* LevelSequenceActor = Cast<ALevelSequenceActor>(Actor);
			if (LevelSequenceActor->ActorHasTag(TAG_NOSAVERESTORE))
			{
				continue;
			}

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
		RestoreLevel(FoundLevel, StreamingLevelState, RestoredObjects);
	}

	for (UObject* RestoredObject : RestoredObjects)
	{
		IEstSaveRestore::Execute_OnPostRestore(RestoredObject);
	}
}

void UEstSaveStatics::RestoreLevel(const ULevel* Level, FEstLevelState LevelState, TArray<UObject*> &RestoredObjects)
{
	// Restore all actors
	for (const FEstActorState ActorState : LevelState.ActorStates)
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
	for (const FEstSequenceState SequenceState : LevelState.SequenceStates)
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
	for (UActorComponent* Component : Parent->GetComponentsByClass(ComponentState.ComponentClass))
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
		if (LevelSequenceActor->GetFName() == SequenceState.ActorName)
		{
			const bool bIsAtStart = SequenceState.FrameNumber == 0;
			const bool bIsAtEnd = SequenceState.FrameNumber >= LevelSequenceActor->SequencePlayer->GetEndTime().Time.FrameNumber.Value;

			LevelSequenceActor->SequencePlayer->SetPlayRate(SequenceState.PlayRate);

			// Don't explicitly set the position if at start
			// this can cause frames to get dropped (and events on frame 0 skipped)
			if (!bIsAtStart)
			{
				LevelSequenceActor->SequencePlayer->PlayToFrame(FFrameTime(SequenceState.FrameNumber));
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

struct FEstSaveGameArchive : public FObjectAndNameAsStringProxyArchive
{
	FEstSaveGameArchive(FArchive& InInnerArchive)
		: FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
	{
		ArIsSaveGame = true;
		ArNoDelta = true;
	}
};

void UEstSaveStatics::RestoreLowLevel(UObject* Object, TArray<uint8> Bytes)
{
	FMemoryReader MemoryReader(Bytes, true);
	FEstSaveGameArchive Ar(MemoryReader);
	Object->Serialize(Ar);
}

void UEstSaveStatics::SerializeLowLevel(UObject* Object, TArray<uint8>& InBytes)
{
	FMemoryWriter MemoryWriter(InBytes, true);
	FEstSaveGameArchive Ar(MemoryWriter);
	Object->Serialize(Ar);
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
