#include "EstGameplayStatics.h"
#include "EstCore.h"
#include "EstImpactEffect.h"
#include "EstImpactManifest.h"
#include "EstPlayer.h"
#include "EstPlayerController.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Components/SkinnedMeshComponent.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerInput.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "AudioDevice.h"
#include "EstGameMode.h"
#include "EstGameInstance.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "EstNoPickupComponent.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Components/DirectionalLightComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Public/ComponentReregisterContext.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"

FRotator UEstGameplayStatics::RandomProjectileSpread(FRotator InRot, float MaxSpread)
{
	// The code below takes the pitch and yaw of the input FRotator, and adjusts them by a random number ranging from -MaxSpread to +MaxSpread. Roll is untouched.
	return FRotator(InRot.Pitch + FMath::FRandRange(-MaxSpread, MaxSpread), InRot.Yaw + FMath::FRandRange(-MaxSpread, MaxSpread), InRot.Roll);
}

struct FEstImpactEffect UEstGameplayStatics::FindImpactEffect(const UEstImpactManifest* ImpactEffects, const UPhysicalMaterial* PhysicalMaterial)
{
	if (PhysicalMaterial != nullptr && ImpactEffects != nullptr && ImpactEffects->ImpactEffectMap.Contains(PhysicalMaterial))
	{
		return ImpactEffects->ImpactEffectMap[PhysicalMaterial];
	}

	return FEstImpactEffect::None;
}

FName UEstGameplayStatics::FindClosestBoneName(USceneComponent* Component, FVector Location)
{
	const USkinnedMeshComponent *SkinnedMesh = Cast<USkinnedMeshComponent>(Component);
	if (SkinnedMesh != nullptr)
	{
		return SkinnedMesh->FindClosestBone(Location);
	}

	return NAME_None;
}

void UEstGameplayStatics::DeployImpactEffect(const FEstImpactEffect ImpactEffect, const FVector ImpactPoint, const FVector ImpactNormal, USceneComponent* Component, const float Scale, USoundAttenuation* AttenuationOverride)
{
	ensureAlways(TEXT("Error: DeployImpactEffect called with null Component parameter"));

	const FVector Position = ImpactPoint;
	const FVector Normal = -ImpactNormal;

	FName ClosestBoneName = FindClosestBoneName(Component, Position);

	const EAttachLocation::Type AttachLocation = EAttachLocation::KeepWorldPosition;

	if (ImpactEffect.Sound != nullptr)
	{
		UGameplayStatics::SpawnSoundAttached(ImpactEffect.Sound, Component, ClosestBoneName, Position, AttachLocation, false, Scale, 1.f, 0.f, AttenuationOverride);
	}

	if (ImpactEffect.ParticleSystem != nullptr)
	{
		UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(ImpactEffect.ParticleSystem, Component, ClosestBoneName, Position, ImpactNormal.Rotation(), FVector(Scale), AttachLocation);
		if (ParticleSystemComponent != nullptr)
		{
			// Always draw particles behind (fixes sorting issues with water)
			ParticleSystemComponent->SetTranslucentSortPriority(-1);
		}
	}

	if (ImpactEffect.ParticleSystemDebris != nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(Component, ImpactEffect.ParticleSystemDebris, Position, ImpactNormal.Rotation(), FVector(Scale));
	}
}

void UEstGameplayStatics::DeployImpactEffectDelayed(const FEstImpactEffect ImpactEffect, const FVector ImpactPoint, const FVector ImpactNormal, class USceneComponent* Component, const float Delay, const float Scale, class USoundAttenuation* AttenuationOverride)
{
	if (FMath::IsNearlyZero(Delay))
	{
		DeployImpactEffect(ImpactEffect, ImpactPoint, ImpactNormal, Component, Scale, AttenuationOverride);
		return;
	}

	TWeakObjectPtr<USceneComponent> ComponentPointer = Component;

	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([ImpactEffect, ImpactPoint, ImpactNormal, ComponentPointer, Scale, AttenuationOverride]
	{
		if (ComponentPointer.IsValid())
		{
			DeployImpactEffect(ImpactEffect, ImpactPoint, ImpactNormal, ComponentPointer.Get(), Scale, AttenuationOverride);
		}
	});

	FTimerHandle Handle;
	Component->GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, Delay, false);
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

	return Actor;
}

void UEstGameplayStatics::ListActionMappings(const APlayerController* PlayerController, TArray<FName> &SortedActionNames, TMap<FName, FInputChord> &Mappings)
{
	check(PlayerController != nullptr);
	check(PlayerController->PlayerInput != nullptr);

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

	Mappings.GetKeys(SortedActionNames);
	SortedActionNames.Sort([](const FName& LHS, const FName& RHS)
	{
		return LHS < RHS;
	});
}

void UEstGameplayStatics::ListAxisMappings(const APlayerController* PlayerController, TArray<FName>& SortedAxisNames, TMap<FName, FKey>& Mappings)
{
	check(PlayerController != nullptr);
	check(PlayerController->PlayerInput != nullptr);

	for (const FInputAxisKeyMapping Mapping : PlayerController->PlayerInput->AxisMappings)
	{
		if (Mapping.Key.IsGamepadKey())
		{
			continue;
		}

		if (Mapping.Key == EKeys::Escape)
		{
			continue;
		}

		Mappings.Add(Mapping.AxisName, Mapping.Key);
	}

	Mappings.GetKeys(SortedAxisNames);
	SortedAxisNames.Sort([](const FName& LHS, const FName& RHS)
	{
		return LHS < RHS;
	});
}

FKey UEstGameplayStatics::FindBestKeyForAction(APlayerController* PlayerController, FName ActionName, bool bForGamepad)
{
	check(PlayerController != nullptr);
	check(PlayerController->PlayerInput != nullptr);

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
	check(PlayerController != nullptr);
	check(PlayerController->PlayerInput != nullptr);

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
		UE_LOG(LogTemp, Log, TEXT("Removing mapping for action %s and key %s"), *Remove.ActionName.ToString(), *Remove.Key.ToString());
		PlayerController->PlayerInput->RemoveActionMapping(Remove);
	}

	UE_LOG(LogTemp, Log, TEXT("Adding mapping for action %s and key %s"), *ActionName.ToString(), *InputChord.Key.ToString());
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
		UE_LOG(LogTemp, Log, TEXT("Removing mapping for action %s and axis %s"), *Remove.AxisName.ToString(), *Remove.Key.ToString());
		PlayerController->PlayerInput->RemoveAxisMapping(Remove);
	}

	UE_LOG(LogTemp, Log, TEXT("Adding mapping for action %s and axis %s"), *AxisName.ToString(), *InputKey.ToString());
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

	if (FParse::Param(FCommandLine::Get(), TEXT("lowendmachine")))
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

const TArray<ULevelStreaming*> UEstGameplayStatics::GetStreamingLevels(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetStreamingLevels();
	}

	return TArray<ULevelStreaming*>();
}

bool UEstGameplayStatics::IsLocationInsideActor(FVector Location, AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	return Actor->GetComponentsBoundingBox().IsInsideOrOn(Location);
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

AActor* UEstGameplayStatics::FindActorBySaveIdInWorld(UObject* WorldContextObject, FGuid SaveId)
{
	for (TActorIterator<AActor> Actor(WorldContextObject->GetWorld(), AActor::StaticClass()); Actor; ++Actor)
	{
		if (!Actor->Implements<UEstSaveRestore>())
		{
			continue;
		}

		const FGuid ActorSaveId = IEstSaveRestore::Execute_GetSaveId(*Actor);
		if (ActorSaveId.IsValid() && ActorSaveId == SaveId)
		{
			return *Actor;
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

bool UEstGameplayStatics::CanHumanPickUpActor(ACharacter* Character, AActor * ActorToHold, float MaxMass, float MaxRadius)
{
	if (Character == nullptr)
	{
		UE_LOG(LogEstGeneral, Warning, TEXT("Picking up character is null"));
		return false;
	}

	if (ActorToHold == nullptr)
	{
		UE_LOG(LogEstGeneral, Warning, TEXT("Can't pick up actor as it is null"));
		return false;
	}

	if (ActorToHold->GetComponentByClass(UEstNoPickupComponent::StaticClass()) != nullptr)
	{
		UE_LOG(LogEstGeneral, Warning, TEXT("Can't pick up actor as it has a no pickup component"));
		return false;
	}

	UPrimitiveComponent* PrimitiveToHold = Cast<UPrimitiveComponent>(ActorToHold->GetRootComponent());
	if (PrimitiveToHold == nullptr)
	{
		UE_LOG(LogEstGeneral, Warning, TEXT("Can't pick up actor as no root primitive"));
		return false;
	}

	if (!PrimitiveToHold->IsSimulatingPhysics())
	{
		UE_LOG(LogEstGeneral, Warning, TEXT("Can't pick up actor as not simulating physics"));
		return false;
	}

	// If we're standing on the object
	if (PrimitiveToHold == Character->GetMovementBase())
	{
		DrawDebugString(ActorToHold->GetWorld(), FVector::ZeroVector, TEXT("Being stood on"), ActorToHold, FColor::White, DEBUG_PERSIST_TIME, true);
		return false;
	}

	if (PrimitiveToHold->GetMass() > MaxMass)
	{
		const FString TooHeavyString = FString::Printf(TEXT("Too heavy (max: %.2fkg, actual: %.2fkg)"), MaxMass, PrimitiveToHold->GetMass());
		DrawDebugString(ActorToHold->GetWorld(), FVector::ZeroVector, TooHeavyString, ActorToHold, FColor::White, DEBUG_PERSIST_TIME, true);
		return false;
	}

	if (PrimitiveToHold->Bounds.SphereRadius > MaxRadius)
	{
		const FString TooBigString = FString::Printf(TEXT("Too big (max: %.2f, actual: %.2f)"), MaxRadius, PrimitiveToHold->Bounds.SphereRadius);
		DrawDebugString(ActorToHold->GetWorld(), FVector::ZeroVector, TooBigString, ActorToHold, FColor::White, DEBUG_PERSIST_TIME, true);
		return false;
	}

	UEstCarryableUserData* CarryableUserData = GetCarryableUserDataFromMesh(PrimitiveToHold);
	if (CarryableUserData != nullptr && !CarryableUserData->bCanPlayerPickUp)
	{
		UE_LOG(LogEstGeneral, Warning, TEXT("Can't pick up actor as asset contains carryable user data stating it cannot be picked up"));
		return false;
	}

	return true;
}

void UEstGameplayStatics::TraceBullet(const USceneComponent* SourceComponent, const FVector ExitLocation, const FRotator ExitRotation, const float MaxSpread, const FOnBulletHitDelegate &OnBulletHit, FRotator &AdjustedRotation, const float MaxDistance)
{
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	const AActor* Weapon = SourceComponent->GetOwner();
	if (Weapon != nullptr)
	{
		QueryParams.AddIgnoredActor(Weapon);
		const AActor* Owner = Weapon->GetOwner();
		if (Owner != nullptr)
		{
			QueryParams.AddIgnoredActor(Owner);
		}
	}

	AdjustedRotation = RandomProjectileSpread(ExitRotation, MaxSpread);

	FVector TraceStart = ExitLocation;
	const FVector TraceEnd = ExitLocation + (AdjustedRotation.Vector() * MaxDistance);

	TArray<EPhysicalSurface> PassThroughSurfaces;
	PassThroughSurfaces.Add(SURFACE_TYPE_FOLIAGE);
	PassThroughSurfaces.Add(SURFACE_TYPE_METAL_MESH);
	PassThroughSurfaces.Add(SURFACE_TYPE_WATER);
	PassThroughSurfaces.Add(SURFACE_TYPE_FLESH);

	TArray<FHitResult> HitResults;

	int32 NumIterations = 0;
	do
	{
		FHitResult HitResult;
		if (!SourceComponent->GetWorld()->LineTraceSingleByProfile(HitResult, TraceStart, TraceEnd, PROFILE_BULLET, QueryParams))
		{
			// Only continue if blocking hit
			return;
		}

		HitResults.Add(HitResult);
		
		// Execute delegate with hits
		OnBulletHit.Execute(HitResults);

		// Only continue if we got a physical material
		if (!HitResult.PhysMaterial.IsValid())
		{
			return;
		}

		// Get the surface type from this physical material
		const EPhysicalSurface PhysicalSurface = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
		if (!PassThroughSurfaces.Contains(PhysicalSurface))
		{
			// Only continue if we can pass through this surface
			return;
		}

		// Set up trace for next iteration
		TraceStart = HitResult.Location;
		QueryParams.AddIgnoredActor(HitResult.GetActor());
	} while (NumIterations++ < 4); // Go through at most 4 surfaces
}

AEstPlayer* UEstGameplayStatics::GetEstPlayerPawn(const UObject* WorldContextObject, int32 PlayerIndex)
{
	return Cast<AEstPlayer>(UGameplayStatics::GetPlayerPawn(WorldContextObject, PlayerIndex));
}

void UEstGameplayStatics::SetHighlightState(AActor* Actor, bool bIsHighlighted)
{
	if (Actor == nullptr)
	{
		return;
	}

	TArray<UMeshComponent*> MeshComponents;
	Actor->GetComponents(MeshComponents);
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		MeshComponent->SetScalarParameterValueOnMaterials(HIGHLIGHT_MATERIAL_PARAMETER, bIsHighlighted ? 1.f : 0.f);
	}
}

void UEstGameplayStatics::FindAllLivingActorsOfClasses(const UObject* WorldContextObject, TSet<TSubclassOf<AActor>> ActorClasses, TSet<AActor*>& OutActors, FName TagFilter)
{
	QUICK_SCOPE_CYCLE_COUNTER(UEstGameplayStatics_FindAllLivingActorsOfClasses);
	OutActors.Reset();

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	// We do nothing if no is classes provided, rather than giving ALL actors!
	if (ActorClasses.Num() > 0 && World)
	{
		for (TActorIterator<AActor> It(World, AActor::StaticClass()); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor->IsPendingKill())
			{
				continue;
			}

			if (TagFilter != NAME_None && !Actor->ActorHasTag(TagFilter))
			{
				continue;
			}

			if (ActorClasses.Contains(Actor->GetClass()) && !Actor->ActorHasTag(TAG_DEAD))
			{
				OutActors.Add(Actor);
			}
		}
	}
}

FRotator UEstGameplayStatics::LocalRotationFromWorldDirection(FVector WorldDirection, FTransform WorldTransform)
{
	return UKismetMathLibrary::InverseTransformRotation(WorldTransform, UKismetMathLibrary::MakeRotFromX(WorldDirection));
}

int32 UEstGameplayStatics::RoundToNearestIntegerIncrement(float Value, int32 Increment)
{
	return FMath::RoundToInt(Value / (float)Increment) * Increment;
}

TArray<AActor*> UEstGameplayStatics::SortActorArrayByClosest(AActor * SelfActor, TArray<AActor*> Actors)
{
	Algo::Sort(Actors, [&SelfActor](const AActor* LHS, const AActor* RHS)
	{
		return SelfActor->GetDistanceTo(LHS) < SelfActor->GetDistanceTo(RHS);
	});

	return Actors;
}

TArray<AActor*> UEstGameplayStatics::FilterActorArrayByVisible(TArray<AActor*> Actors, TArray<UClass*> AlwaysAllowed)
{
	return Actors.FilterByPredicate([&AlwaysAllowed](const AActor* Actor)
	{
		if (Actor == nullptr)
		{
			return false;
		}

		if (AlwaysAllowed.Contains(Actor->GetClass()))
		{
			return true;
		}

		return !Actor->bHidden;
	});
}

void UEstGameplayStatics::SetLightMapResolution(UStaticMeshComponent *Component, bool bNewOverrideLightMapRes, int32 NewOverriddenLightMapRes)
{
	if (Component == nullptr)
	{
		return;
	}

	Component->bOverrideLightMapRes = bNewOverrideLightMapRes;
	Component->OverriddenLightMapRes = NewOverriddenLightMapRes;
#if WITH_EDITOR
	Component->InvalidateLightingCache();
	Component->OnStaticMeshChanged().Broadcast(Component);
#endif
}

void UEstGameplayStatics::SetCsmSettings(UDirectionalLightComponent *Component, float DynamicShadowDistanceStationaryLight, float CascadeDistributionExponent, int32 DynamicShadowCascades, bool bUseInsetShadowsForMovableObjects)
{
	if (Component == nullptr)
	{
		return;
	}

	Component->DynamicShadowDistanceStationaryLight = DynamicShadowDistanceStationaryLight;
	Component->CascadeDistributionExponent = CascadeDistributionExponent;
	Component->DynamicShadowCascades = DynamicShadowCascades;
	Component->bUseInsetShadowsForMovableObjects = bUseInsetShadowsForMovableObjects;
	Component->MarkRenderStateDirty();
}

float UEstGameplayStatics::GetCameraFadeAmount(APlayerCameraManager * PlayerCameraManager)
{
	if (PlayerCameraManager == nullptr)
	{
		return 0.f;
	}

	return PlayerCameraManager->FadeAmount;
}

UEstCarryableUserData* UEstGameplayStatics::GetCarryableUserDataFromMesh(UPrimitiveComponent* PrimitiveComponent)
{
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(PrimitiveComponent);
	if (StaticMeshComponent != nullptr && StaticMeshComponent->GetStaticMesh() != nullptr)
	{
		return StaticMeshComponent->GetStaticMesh()->GetAssetUserData<UEstCarryableUserData>();
	}

	USkinnedMeshComponent* SkinnedMeshComponent = Cast<USkinnedMeshComponent>(PrimitiveComponent);
	if (SkinnedMeshComponent != nullptr && SkinnedMeshComponent->SkeletalMesh != nullptr)
	{
		return SkinnedMeshComponent->SkeletalMesh->GetAssetUserData<UEstCarryableUserData>();
	}

	return nullptr;
}

void UEstGameplayStatics::MarkRenderStateDirty(UActorComponent* ActorComponent)
{
	if (ActorComponent != nullptr)
	{
		ActorComponent->MarkRenderStateDirty();
	}
}

float UEstGameplayStatics::GetDuration(UAudioComponent* AudioComponent)
{
	if (AudioComponent == nullptr || AudioComponent->Sound == nullptr)
	{
		return 0.f;
	}

	return AudioComponent->Sound->Duration;
}

float UEstGameplayStatics::GetPlayPositionWithinLoop(UAudioComponent* AudioComponent, float PlayPosition)
{
	return FMath::Fmod(PlayPosition, UEstGameplayStatics::GetDuration(AudioComponent));
}

bool UEstGameplayStatics::IsSuitableStopPoint(UAudioComponent* AudioComponent, float PlayPosition)
{
	if (AudioComponent == nullptr || AudioComponent->Sound == nullptr)
	{
		return true;
	}

	return UEstGameplayStatics::GetPlayPositionWithinLoop(AudioComponent, PlayPosition) < 0.5f;
}

bool UEstGameplayStatics::IsLooping(UAudioComponent* AudioComponent)
{
	if (AudioComponent == nullptr || AudioComponent->Sound == nullptr)
	{
		return false;
	}

	return AudioComponent->Sound->IsLooping();
}

void UEstGameplayStatics::PingUrl(FString Url, const FPingUrlResultDelegate &Callback)
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("GET");
	HttpRequest->SetURL(Url);
	HttpRequest->OnProcessRequestComplete().BindStatic([](FHttpRequestPtr RefHttpRequest, FHttpResponsePtr RefHttpResponse, bool bSucceeded, FPingUrlResultDelegate RefCallback)
	{
		if (!bSucceeded || !RefHttpResponse.IsValid())
		{
			RefCallback.ExecuteIfBound(false);
			return;
		}

		RefCallback.ExecuteIfBound(RefHttpResponse->GetResponseCode() < 500);
	}, Callback);
	HttpRequest->ProcessRequest();
}