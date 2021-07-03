#pragma once

#include "HAL/IConsoleManager.h"
#include "Engine/Classes/Engine/EngineTypes.h"
#include "Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/InputCore/Classes/InputCoreTypes.h"
#include "Framework/Commands/InputChord.h"
#include "GameFramework/Actor.h"
#include "Physics/EstImpactEffect.h"
#include "UserData/EstCarryableUserData.h"
#include "JsonObjectConverter.h"
#include "EstConstants.h"
#include "EstGameplayStatics.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBulletHitDelegate, const TArray<FHitResult>&, HitResults);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPingUrlResultDelegate, bool, bSuccess);

UCLASS()
class ESTCORE_API UEstGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Calculate a new rotation with a random spread under MaxSpread. */
	UFUNCTION(BlueprintPure, Category = Projectiles)
	static FRotator RandomProjectileSpread(FRotator InRot, float MaxSpread);

	/** Deploy impact effects at a specified location. */
	UFUNCTION(BlueprintCallable, Category = Projectiles)
	static void DeployImpactEffect(const FEstImpactEffect ImpactEffect, const FVector ImpactPoint, const FVector ImpactNormal, class USceneComponent* Component, const float Scale = 1.0f, class USoundAttenuation* AttenuationOverride = nullptr);

	/** Deploy impact effects at a specified location with a delay. */
	UFUNCTION(BlueprintCallable, Category = Projectiles)
	static void DeployImpactEffectDelayed(const FEstImpactEffect ImpactEffect, const FVector ImpactPoint, const FVector ImpactNormal, class USceneComponent* Component, const float Delay, const float Scale = 1.0f, class USoundAttenuation* AttenuationOverride = nullptr);

	/** Find physical impact effects based on a material with an optional default. */
	UFUNCTION(BlueprintPure, Category = Projectiles)
	static struct FEstImpactEffect FindImpactEffect(const class UEstImpactManifest* ImpactEffects, const class UPhysicalMaterial* PhysicalMaterial);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static class UPhysicalMaterial* GetPhysicalMaterial(const FHitResult &HitResult);

	UFUNCTION(BlueprintPure, Category = Projectiles)
	static FName FindClosestBoneName(USceneComponent * Component, FVector Location);

	/** Pause or unpause all local player controllers. */
	UFUNCTION(BlueprintCallable, Category = Game)
	static void SetPause(bool bIsPaused);

	/** Send a command to all local player controllers */
	UFUNCTION(BlueprintCallable, Category = Game)
	static void ConsoleCommand(const FString& Command);

	UFUNCTION(BlueprintCallable, Category = Game)
	static AActor* MoveActorToLevel(AActor* Actor, ULevel* Level);

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (HidePin = "PlayerController", DefaultToSelf = "PlayerController"))
	static void ListActionMappings(const APlayerController* PlayerController, TArray<FName> &SortedActionNames, TMap<FName, FInputChord> &Mappings, bool bForGamepad);

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (HidePin = "PlayerController", DefaultToSelf = "PlayerController"))
	static void ListAxisMappings(const APlayerController* PlayerController, TArray<FName> &SortedAxisNames, TMap<FName, FKey> &Mappings, bool bForGamepad);

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (HidePin = "PlayerController", DefaultToSelf = "PlayerController"))
	static FKey FindBestKeyForAction(APlayerController* PlayerController, FName ActionName, bool bForGamepad);

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (HidePin = "PlayerController", DefaultToSelf = "PlayerController"))
	static FKey FindBestKeyForAxis(APlayerController* PlayerController, FName AxisNames, bool bForGamepad);

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (HidePin = "PlayerController", DefaultToSelf = "PlayerController"))
	static void AddAxisMapping(APlayerController* PlayerController, FName AxisName, FKey InputKey, float Scale);

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (HidePin = "PlayerController", DefaultToSelf = "PlayerController"))
	static void AddActionMapping(APlayerController* PlayerController, FName ActionName, FInputChord InputKey);

	/** Get the Estranged player controller for a specified player index. */
	UFUNCTION(BlueprintPure, Category = "Game", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static class AEstPlayerController* GetEstPlayerController(UObject* WorldContextObject, int32 PlayerIndex);

	UFUNCTION(BlueprintPure, Category = "Game", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static bool IsEditor(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Game")
	static bool IsSlowerMachine();

	UFUNCTION(BlueprintPure, Category = "Game")
	static bool IsDesktopPlatform() { return PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC; }

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static AActor* CreateTransientActor(UObject* WorldContextObject, class UClass* ActorClass);

	UFUNCTION(BlueprintCallable, Category = "Saving")
	static TArray<FString> ListSaveGames(FString Directory, FString Filter);

	UFUNCTION(BlueprintCallable, Category = "Debugging")
	static TArray<FString> GetValidMemoryDumpPaths();

	UFUNCTION(BlueprintPure, Category = "Game", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static const TArray<class ULevelStreaming*> GetStreamingLevels(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = Math)
	static bool IsLocationInsideActor(FVector Location, AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "Input")
	static bool IsUsingGamepad(const APawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void SetTransientMasterVolume(UObject* WorldContextObject, const float InTransientMasterVolume);

	UFUNCTION(BlueprintPure, Category = "Saving")
	static FString GenerateSaveFileName() { return FDateTime::Now().ToString(); };

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static AActor* FindActorBySaveIdInWorld(UObject* WorldContextObject, FGuid SaveId);

	template <typename TActor>
	static TActor* FindActorBySaveIdInWorld(UObject* WorldContextObject, FGuid SaveId) { return Cast<TActor>(FindActorBySaveIdInWorld(WorldContextObject, SaveId)); };

	UFUNCTION(BlueprintCallable, Category = "Game", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static AActor* FindActorByName(UObject* WorldContextObject, FName ActorName, UClass* ActorClass = nullptr);

	UFUNCTION(BlueprintPure, Category = "Game", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static class AEstGameMode* GetEstGameMode(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Game", meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static class UEstGameInstance* GetEstGameInstance(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static bool CanHumanPickUpActor(ACharacter* Character, AActor * ActorToHold, float MaxMass = 100.f, float MaxRadius = 100.f);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static void TraceBullet(const USceneComponent* SourceComponent, const FVector ExitLocation, const FRotator ExitRotation, const float MaxSpread, const FOnBulletHitDelegate &OnBulletHit, FRotator &AdjustedRotation, const float MaxDistance = 100000.f);

	UFUNCTION(BlueprintPure, Category = "Game", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static class AEstPlayer* GetEstPlayerPawn(const UObject* WorldContextObject, int32 PlayerIndex);

	UFUNCTION(BlueprintPure, Category = "Platform")
	static bool HasGraphicsMenu() { return PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX; };

	UFUNCTION(BlueprintCallable, Category = "Game")
	static void SetHighlightState(AActor* Actor, bool bIsHighlighted);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static void NewGuidIfInvalid(UPARAM(ref) FGuid &Guid) { Guid = Guid.IsValid() ? Guid : FGuid::NewGuid(); };

	UFUNCTION(BlueprintCallable, Category = "Utilities", meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "ActorClasses", DynamicOutputParam = "OutActors"))
	static void FindAllLivingActorsOfClasses(const UObject* WorldContextObject, TSet<TSubclassOf<AActor>> ActorClasses, TSet<AActor*>& OutActors, FName TagFilter);

	UFUNCTION(BlueprintPure, Category = Math)
	static FRotator LocalRotationFromWorldDirection(FVector WorldDirection, FTransform WorldTransform);

	UFUNCTION(BlueprintPure, Category = Math)
	static int32 RoundToNearestIntegerIncrement(float Value, int32 Increment);

	UFUNCTION(BlueprintCallable, Category = Math)
	static TArray<AActor*> SortActorArrayByClosest(AActor* SelfActor, TArray<AActor*> Actors);

	UFUNCTION(BlueprintCallable, Category = Math)
	static TArray<AActor*> FilterActorArrayByVisible(TArray<AActor*> Actors, TArray<UClass*> AlwaysAllowed);

	UFUNCTION(BlueprintCallable, Category = Editor)
	static void SetLightMapResolution(class UStaticMeshComponent* Component, bool bNewOverrideLightMapRes, int32 NewOverriddenLightMapRes);

	UFUNCTION(BlueprintCallable, Category = Editor)
	static void SetCsmSettings(class UDirectionalLightComponent* Component, float DynamicShadowDistanceStationaryLight, float CascadeDistributionExponent, int32 DynamicShadowCascades, bool bUseInsetShadowsForMovableObjects);

	UFUNCTION(BlueprintPure, Category = Player)
	static float GetCameraFadeAmount(class APlayerCameraManager* PlayerCameraManager);

	UFUNCTION(BlueprintCallable, Category = Player)
	static UEstCarryableUserData* GetCarryableUserDataFromMesh(class UPrimitiveComponent* PrimitiveComponent);

	UFUNCTION(BlueprintCallable, Category = Rendering)
	static void MarkRenderStateDirty(class UActorComponent* ActorComponent);

	UFUNCTION(BlueprintPure, Category = Audio)
	static float GetDuration(class UAudioComponent* AudioComponent);

	UFUNCTION(BlueprintPure, Category = Audio)
	static float GetPlayPositionWithinLoop(class UAudioComponent* AudioComponent, float PlayPosition);

	UFUNCTION(BlueprintPure, Category = Audio)
	static bool IsSuitableStopPoint(class UAudioComponent* AudioComponent, float PlayPosition);

	UFUNCTION(BlueprintPure, Category = Audio)
	static bool IsLooping(class UAudioComponent* AudioComponent);

	UFUNCTION(BlueprintCallable, Category = Rendering)
	static void PingUrl(FString Url, const FPingUrlResultDelegate &Callback);

	UFUNCTION(BlueprintPure, Category = Conversion)
	static FString ToString(int64 Integer) { return FString::Printf(TEXT("%lld"), Integer);	}

	UFUNCTION(BlueprintPure, Category = Conversion)
	static FText ToText(int64 Integer) { return FText::AsNumber(Integer); }

	UFUNCTION(BlueprintPure, Category = Conversion)
	static float Divide(int64 Integer, int64 Divisor) { return float(double(Integer) / double(Divisor)); };

	UFUNCTION(BlueprintPure, Category = Internationalization)
	static bool IsUsingPlatformCulture();

	UFUNCTION(BlueprintPure, Category = Internationalization)
	static FString GetGameUserSettingsIniCulture();

	UFUNCTION(BlueprintCallable, Category = Internationalization)
	static void ClearGameUserSettingsIniCulture();

	UFUNCTION(BlueprintPure, Category = Parsing)
	static void ParseVersion(FString Version, TArray<int32> &Components);

	UFUNCTION(BlueprintPure, Category = Project)
	static FString GetProjectVersion();

	UFUNCTION(BlueprintCallable, Category = Platform)
	static void SetPlatformMouseCursorState(bool bShowMouseCursor);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static void SortResolutions(UPARAM(ref) TArray<FIntPoint> &Resolutions);

	UFUNCTION(BlueprintCallable, Category = RenderTexture)
	static void WidgetToTexture(UTextureRenderTarget2D* Texture, class UUserWidget* Widget, const FVector2D &DrawSize, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = RenderTexture)
	static void UpdateResourceImmediate(class UTextureRenderTarget2D* RenderTarget, bool bClearRenderTarget);

	UFUNCTION(BlueprintPure, Category = Health)
	static bool IsActorDead(AActor* Actor);

	UFUNCTION(BlueprintPure, Category = Health)
	static bool IsActorAlive(AActor* Actor) { return !IsActorDead(Actor); }

	UFUNCTION(BlueprintCallable, Category = "ForceFeedback")
	static class UForceFeedbackComponent* SpawnForceFeedbackAttached(class UForceFeedbackEffect* ForceFeedbackEffect, USceneComponent* AttachToComponent);

	UFUNCTION(BlueprintCallable, Category = "ForceFeedback")
	static void ClientPlayForceFeedback(class APlayerController* PlayerController, class UForceFeedbackEffect* ForceFeedbackEffect, FName Tag = NAME_None, bool bLooping = false, bool bIgnoreTimeDilation = false, bool bPlayWhilePaused = false);

	UFUNCTION(BlueprintPure, Category = Input)
	static bool IsAxisKey(FKey Key);

	UFUNCTION(BlueprintCallable, Category = Input)
	static void UpdateCameraManager(class APlayerController* Controller, float DeltaSeconds);
};
