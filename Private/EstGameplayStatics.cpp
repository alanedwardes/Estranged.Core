#include "EstCore.h"
#include "EstGameplayStatics.h"
#include "EstBallistics.h"
#include "EstImpactEffect.h"
#include "EstPlayerController.h"
#include "Internationalization/Internationalization.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"
#include "AudioDevice.h"
#include "EstGameMode.h"
#include "EstGameInstance.h"

FRotator UEstGameplayStatics::RandomProjectileSpread(FRotator InRot, float MaxSpread)
{
	// The code below takes the pitch and yaw of the input FRotator, and adjusts them by a random number ranging from -MaxSpread to +MaxSpread. Roll is untouched.
	return FRotator(InRot.Pitch + FMath::FRandRange(-MaxSpread, MaxSpread), InRot.Yaw + FMath::FRandRange(-MaxSpread, MaxSpread), InRot.Roll);
}

void UEstGameplayStatics::HandleProjectileImpact(UEstBallistics* Ballistics, const FHitResult& ImpactResult)
{
	if (Ballistics == nullptr)
	{
		return;
	}

	// AETODO: Fill in nullptr parameters, look into adding radial damage in the ballistics info
	UGameplayStatics::ApplyPointDamage(ImpactResult.Actor.Get(), Ballistics->Damage, ImpactResult.TraceStart, ImpactResult, nullptr, nullptr, nullptr);

	if (ImpactResult.Component.IsValid())
	{
		const FEstImpactEffect ImpactEffects = FindImpactEffect(Ballistics->ImpactEffects, ImpactResult.PhysMaterial.Get(), Ballistics->DefaultImpactEffect);
		DeployImpactEffect(ImpactEffects, ImpactResult.Location, ImpactResult.Normal, ImpactResult.Component.Get());
	}
}

struct FEstImpactEffect UEstGameplayStatics::FindImpactEffect(const TArray<FEstImpactEffect> ImpactEffects, const UPhysicalMaterial* PhysicalMaterial, const FEstImpactEffect DefaultImpactEffect)
{
	// If we got a dud, use the default FX
	if (PhysicalMaterial == nullptr)
	{
		return DefaultImpactEffect;
	}

	// If we don't have any data loaded, default
	if (ImpactEffects.Num() == 0)
	{
		return DefaultImpactEffect;
	}

	// Loop our currently loaded impact data array,
	// and check if we have a definition for the current
	// physical material.
	for (const FEstImpactEffect ImpactData : ImpactEffects)
	{
		if (ImpactData.PhysicalMaterial)
		{
			if (ImpactData.PhysicalMaterial == PhysicalMaterial)
			{
				return ImpactData;
			}
		}
	}

	// No match, default FX
	return DefaultImpactEffect;
}

void UEstGameplayStatics::DeployImpactEffect(const FEstImpactEffect ImpactEffect, const FVector ImpactPoint, const FVector ImpactNormal, UPrimitiveComponent* Component, const float Scale, USoundAttenuation* AttenuationOverride)
{
	check(Component != nullptr);

	const FVector Position = ImpactPoint;
	const FVector Normal = -ImpactNormal;

	if (ImpactEffect.Decal != nullptr)
	{
		// Skinned meshes require us to attach the decal to a specific part of the skeleton for it to look right.
		const USkinnedMeshComponent *SkinnedMesh = Cast<class USkinnedMeshComponent>(Component);
		if (SkinnedMesh != nullptr)
		{
			// TODO: Figure out how else we could tackle this problem.
			// By attaching the decal to the bone we still experience a lot of 'texture swimming' issues.
			FName BoneName = SkinnedMesh->FindClosestBone(Position);

			UGameplayStatics::SpawnDecalAttached(ImpactEffect.Decal, ImpactEffect.DecalSize, Component, BoneName, Position, Normal.Rotation(), EAttachLocation::KeepWorldPosition);
		}
		else
		{
			UGameplayStatics::SpawnDecalAttached(ImpactEffect.Decal, ImpactEffect.DecalSize, Component, NAME_None, Position, Normal.Rotation(), EAttachLocation::KeepWorldPosition);
		}
	}

	if (ImpactEffect.Sound != nullptr)
	{
		UGameplayStatics::SpawnSoundAttached(ImpactEffect.Sound, Component, NAME_None, Position, EAttachLocation::KeepWorldPosition, false, Scale, 1.f, 0.f, AttenuationOverride);
	}

	if (ImpactEffect.ParticleSystem != nullptr)
	{
		if (Component->IsSimulatingPhysics())
		{
			UGameplayStatics::SpawnEmitterAtLocation(Component, ImpactEffect.ParticleSystem, Position, ImpactNormal.Rotation());
		}
		else
		{
			UGameplayStatics::SpawnEmitterAttached(ImpactEffect.ParticleSystem, Component, NAME_None, Position, ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition);
		}
	}

	if (EST_IN_VIEWPORT)
	{
		// Visualise impact point
		DrawDebugPoint(Component->GetWorld(), Position, 10.f, FColor::Red, false, DEBUG_PERSIST_TIME);
	}
}

void UEstGameplayStatics::SetPause(bool bIsPaused)
{
	if (!GEngine)
	{
		return;
	}

	TArray<APlayerController*> Controllers;
	GEngine->GetAllLocalPlayerControllers(Controllers);

	// Enable or disable transition messages
	GEngine->GameViewport->SetSuppressTransitionMessage(bIsPaused);

	for (APlayerController* Controller : Controllers)
	{
		if (!Controller)
		{
			continue;
		}

		Controller->SetPause(bIsPaused);
	}
}

void UEstGameplayStatics::ConsoleCommand(const FString& Command)
{
	if (!GEngine)
	{
		return;
	}

	TArray<APlayerController*> Controllers;
	GEngine->GetAllLocalPlayerControllers(Controllers);

	for (APlayerController *Controller : Controllers)
	{
		if (!Controller)
		{
			continue;
		}

		Controller->ConsoleCommand(Command);
	}
}

AActor* UEstGameplayStatics::MoveActorToLevel(AActor* Actor, ULevel* Level)
{
	const ULevel* FromLevel = Actor->GetLevel();
	if (FromLevel == Level)
	{
		return Actor;
	}

	const FName FromName = Actor->GetFName();

	Actor->GetWorld()->RemoveActor(Actor, false);
	Actor->Rename(nullptr, Level);
	Level->Actors.Add(Actor);

	FString TagToAdd = TEXT(TAG_FROM_PREFIX) + FromLevel->GetOutermost()->GetName();

	Actor->Tags.Add(FName(*TagToAdd));
	Actor->Tags.Add(TAG_MOVED);

	return Actor;
}

TMap<FName, FInputChord> UEstGameplayStatics::ListActionMappings(APlayerController* PlayerController)
{
	TMap<FName, FInputChord> Mappings;
	for (const FInputActionKeyMapping Mapping : PlayerController->PlayerInput->ActionMappings)
	{
		if (Mapping.Key.IsGamepadKey())
		{
			continue;
		}

		if (Mapping.Key == EKeys::Escape)
		{
			continue;
		}

		const FInputChord InputChord = FInputChord(Mapping.Key, Mapping.bShift, Mapping.bCtrl, Mapping.bAlt, Mapping.bCmd);
		Mappings.Add(Mapping.ActionName, InputChord);
	}
	return Mappings;
}

FKey UEstGameplayStatics::FindBestKeyForAction(APlayerController* PlayerController, FName ActionName, bool bForGamepad)
{
	const TArray<FInputActionKeyMapping> Mappings = PlayerController->PlayerInput->GetKeysForAction(ActionName);
	for (const FInputActionKeyMapping Mapping : Mappings)
	{
		if (bForGamepad && Mapping.Key.IsGamepadKey())
		{
			return Mapping.Key;
		}
		
		if (!bForGamepad && !Mapping.Key.IsGamepadKey())
		{
			return Mapping.Key;
		}
	}

	return EKeys::Invalid;
}

FKey UEstGameplayStatics::FindBestKeyForAxis(APlayerController* PlayerController, FName AxisName, bool bForGamepad)
{
	const TArray<FInputAxisKeyMapping> Mappings = PlayerController->PlayerInput->GetKeysForAxis(AxisName);
	for (const FInputAxisKeyMapping Mapping : Mappings)
	{
		if (bForGamepad && Mapping.Key.IsGamepadKey())
		{
			return Mapping.Key;
		}

		if (!bForGamepad && !Mapping.Key.IsGamepadKey())
		{
			return Mapping.Key;
		}
	}

	return EKeys::Invalid;
}

void UEstGameplayStatics::AddActionMapping(APlayerController* PlayerController, FName ActionName, FInputChord InputChord, bool bForGamepad)
{
	check(PlayerController != nullptr);
	check(PlayerController->PlayerInput != nullptr);

	TArray<FInputActionKeyMapping> MappingsToRemove;
	for (const FInputActionKeyMapping Existing : PlayerController->PlayerInput->ActionMappings)
	{
		if (Existing.ActionName != ActionName)
		{
			continue;
		}

		if (Existing.Key.IsGamepadKey() != bForGamepad)
		{
			continue;
		}

		if (Existing.Key == EKeys::Escape)
		{
			continue;
		}

		MappingsToRemove.Add(Existing);
	}

	for (const FInputActionKeyMapping Remove : MappingsToRemove)
	{
		UE_LOG(LogTemp, Warning, TEXT("Removing mapping for action %s and key %s"), *Remove.ActionName.ToString(), *Remove.Key.ToString());
		PlayerController->PlayerInput->RemoveActionMapping(Remove);
	}

	UE_LOG(LogTemp, Warning, TEXT("Adding mapping for action %s and key %s"), *ActionName.ToString(), *InputChord.Key.ToString());
	PlayerController->PlayerInput->AddActionMapping(FInputActionKeyMapping(ActionName, InputChord.Key));
}

FString UEstGameplayStatics::GetKeyDisplayName(FKey Key)
{
	FString DisplayName = Key.GetDisplayName().ToString();
	DisplayName.ReplaceInline(TEXT("-Axis"), TEXT(""));
	DisplayName.RemoveFromEnd(TEXT("X"));
	DisplayName.RemoveFromEnd(TEXT("Y"));
	return DisplayName;
}

void UEstGameplayStatics::AddAxisMapping(APlayerController* PlayerController, FName AxisName, FKey InputKey, float Scale, bool bForGamepad)
{
	check(PlayerController != nullptr);
	check(PlayerController->PlayerInput != nullptr);

	TArray<FInputAxisKeyMapping> MappingsToRemove;
	for (const FInputAxisKeyMapping Existing : PlayerController->PlayerInput->AxisMappings)
	{
		if (Existing.AxisName != AxisName)
		{
			continue;
		}

		if (Existing.Key.IsGamepadKey() != bForGamepad)
		{
			continue;
		}

		if (Existing.Key == EKeys::Escape)
		{
			continue;
		}

		MappingsToRemove.Add(Existing);
	}

	for (const FInputAxisKeyMapping Remove : MappingsToRemove)
	{
		UE_LOG(LogTemp, Warning, TEXT("Removing mapping for action %s and axis %s"), *Remove.AxisName.ToString(), *Remove.Key.ToString());
		PlayerController->PlayerInput->RemoveAxisMapping(Remove);
	}

	UE_LOG(LogTemp, Warning, TEXT("Adding mapping for action %s and axis %s"), *AxisName.ToString(), *InputKey.ToString());
	PlayerController->PlayerInput->AddAxisMapping(FInputAxisKeyMapping(AxisName, InputKey, Scale));
}

class AEstPlayerController* UEstGameplayStatics::GetEstPlayerController(UObject* WorldContextObject, int32 PlayerIndex)
{
	return Cast<AEstPlayerController>(UGameplayStatics::GetPlayerController(WorldContextObject, PlayerIndex));
}

AActor* UEstGameplayStatics::CreateTransientActor(UObject* WorldContextObject, UClass *ActorClass)
{
	if (WorldContextObject == nullptr || WorldContextObject->GetWorld() == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.ObjectFlags = EObjectFlags::RF_Transient;
	SpawnParameters.OverrideLevel = WorldContextObject->GetWorld()->PersistentLevel;
	SpawnParameters.bAllowDuringConstructionScript = true;
	return WorldContextObject->GetWorld()->SpawnActor(ActorClass, &FTransform::Identity, SpawnParameters);
}

bool UEstGameplayStatics::IsEditor(UObject* WorldContextObject)
{
	if (WorldContextObject == nullptr || WorldContextObject->GetWorld() == nullptr)
	{
		return false;
	}

	return WorldContextObject->GetWorld()->WorldType == EWorldType::Editor || WorldContextObject->GetWorld()->WorldType == EWorldType::EditorPreview;
}

bool UEstGameplayStatics::IsSlowerMachine()
{
	if (FPlatformMisc::NumberOfCoresIncludingHyperthreads() < 4)
	{
		return true;
	}

	if (FEstCoreModule::GetLongAverageFrameRate() < 25.f)
	{
		return true;
	}

	return false;
}

TArray<FString> UEstGameplayStatics::ListSaveGames(FString Directory)
{
	FString SaveGamesFolder = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("SaveGames"), Directory);

	IFileManager &FileManager = IFileManager::Get();

	TArray<FString> AllSaveGamePaths;
	FileManager.FindFiles(AllSaveGamePaths, *SaveGamesFolder, TEXT(".sav"));

	// Sort so latest files are first
	AllSaveGamePaths.Sort([&FileManager, SaveGamesFolder](const FString& LHS, const FString& RHS)
	{
		return FileManager.GetTimeStamp(*FPaths::Combine(SaveGamesFolder, *LHS)) > FileManager.GetTimeStamp(*FPaths::Combine(SaveGamesFolder, *RHS));
	});

	TArray<FString> ValidSaveGames;
	for (const FString SaveGamePath : AllSaveGamePaths)
	{
		ValidSaveGames.Add(FPaths::GetBaseFilename(SaveGamePath));
	}

	return ValidSaveGames;
}

TArray<FString> UEstGameplayStatics::GetValidMemoryDumpPaths()
{
	FString CrashFolder = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("Crashes"));

	TArray<FString> AllMemoryDumpPaths;
	IFileManager::Get().FindFilesRecursive(AllMemoryDumpPaths, *CrashFolder, *FGenericCrashContext::UE4MinidumpName, true, false);

	TArray<FString> ValidMemoryDumpPaths;
	for (const FString MemoryDumpPath : AllMemoryDumpPaths)
	{
		const int64 MemoryDumpSize = IFileManager::Get().FileSize(*MemoryDumpPath);

		// Ensure not too small
		if (MemoryDumpSize < 1024)
		{
			EST_DEBUG(FString::Printf(TEXT("Skipping dump %s as it is too small"), *MemoryDumpPath));
			continue;
		}

		// Ensure not bigger than we can upload
		if (MemoryDumpSize > (1024 * 1024 * 9))
		{
			EST_DEBUG(FString::Printf(TEXT("Skipping dump %s as it is too big"), *MemoryDumpPath));
			ValidMemoryDumpPaths.Add(MemoryDumpPath);
		}

		// Ensure not more than a month old
		const FDateTime ModifiedTime = IFileManager::Get().GetTimeStamp(*MemoryDumpPath);
		if (ModifiedTime - FDateTime::Now() > FTimespan::FromDays(30))
		{
			EST_DEBUG(FString::Printf(TEXT("Skipping dump %s as it is too old"), *MemoryDumpPath));
			continue;
		}

		ValidMemoryDumpPaths.Add(MemoryDumpPath);
	}

	return ValidMemoryDumpPaths;
}

bool UEstGameplayStatics::IsLocationInsideActor(FVector Location, AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	return Actor->GetComponentsBoundingBox().IsInsideOrOn(Location);
}

TArray<FString> UEstGameplayStatics::GetAllCultures()
{
	TArray<FString> LocalizationPaths;
	LocalizationPaths += FPaths::GetGameLocalizationPaths();
	TArray<FCultureRef> AllCultureRefs;
	FInternationalization::Get().GetCulturesWithAvailableLocalization(LocalizationPaths, AllCultureRefs, false);

	TArray<FString> AllCutureNames;
	for (const FCultureRef CultureRef : AllCultureRefs)
	{
		AllCutureNames.Add(CultureRef->GetName());
	}
	return AllCutureNames;
}

bool UEstGameplayStatics::IsUsingGamepad(const APawn* Pawn)
{
	const AEstPlayerController* Controller = Cast<AEstPlayerController>(Pawn->Controller);
	if (Controller == nullptr)
	{
		return false;
	}

	return Controller->bIsUsingGamepad;
}

void UEstGameplayStatics::SetTransientMasterVolume(UObject* WorldContextObject, const float InTransientMasterVolume)
{
	if (WorldContextObject == nullptr)
	{
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (FAudioDevice* GameInstanceAudioDevice = World->GetAudioDevice())
	{
		GameInstanceAudioDevice->SetTransientMasterVolume(InTransientMasterVolume);
	}
}

AActor* UEstGameplayStatics::SpawnActor(UObject* WorldContextObject, UClass* ActorClass, FName ActorName, const FTransform &ActorTransform)
{
	EST_DEBUG(FString::Printf(TEXT("Spawning actor: %s"), *ActorName.ToString()));

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = ActorName;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return WorldContextObject->GetWorld()->SpawnActor<AActor>(ActorClass, ActorTransform, SpawnParameters);
}

AActor* UEstGameplayStatics::FindActorByNameAndClassInLevel(const ULevel* Level, FName ActorName, UClass* ActorClass)
{
	for (AActor* Actor : Level->Actors)
	{
		if (!Actor)
		{
			continue;
		}

		if (Actor->GetClass() != ActorClass)
		{
			continue;
		}

		if (Actor->GetFName() == ActorName)
		{
			return Actor;
		}
	}

	return nullptr;
}

AActor* UEstGameplayStatics::FindActorByName(UObject* WorldContextObject, FName ActorName, UClass* ActorClass)
{
	if (ActorName == NAME_None)
	{
		return nullptr;
	}

	for (TActorIterator<AActor> Actor(WorldContextObject->GetWorld(), ActorClass ? ActorClass : AActor::StaticClass()); Actor; ++Actor)
	{
		if (Actor->GetFName() == ActorName)
		{
			return *Actor;
		}
	}

	return nullptr;
}

AEstGameMode* UEstGameplayStatics::GetEstGameMode(UObject* WorldContextObject)
{
	return Cast<AEstGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
}

UEstGameInstance* UEstGameplayStatics::GetEstGameInstance(UObject* WorldContextObject)
{
	return Cast<UEstGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
}
