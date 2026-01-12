// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PhysicsVolume.h"
#include "Volumes/EstWaterManifest.h"
#include "EstWaterVolume.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API AEstWaterVolume : public APhysicsVolume
{
	GENERATED_UCLASS_BODY()

public:
	virtual void PostInitializeComponents() override;

	virtual void ActorEnteredVolume(class AActor* Other) override;
	virtual void ActorLeavingVolume(class AActor* Other) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void SetPlayerPaddling(bool bEnabled);
	virtual void SetPlayerImmersed(bool bEnabled);

	virtual void ManifestChanged();

#if WITH_EDITORONLY_DATA
	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	bool bUpdateDebugOverlays;

	void OnSelectionChanged(UObject* NewSelection);

	bool bSelectedInEditor;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void SetMaterialParameters();

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite, Interp)
	class UEstWaterManifest* Manifest;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* AboveWaterMesh;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* BelowWaterMesh;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class USphereComponent* WaveExcluder;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	bool bUseWaveExcluder;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	float ExcluderFadeRadius;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite, Interp)
	float WaveIntensity = 1.f;

	UPROPERTY(Category = "Water | Debug", EditAnywhere, BlueprintReadWrite)
	bool bDebugDrawCPUWaves;

	UPROPERTY(Category = "Water | Debug", EditAnywhere, BlueprintReadWrite)
	float DebugGridSize = 1000.f;

	UPROPERTY(Category = "Water | Debug", EditAnywhere, BlueprintReadWrite)
	float DebugGridSpacing = 100.f;

	UFUNCTION(BlueprintPure, Category = "Water")
	virtual FVector GetSurface();

	UFUNCTION(BlueprintPure, Category = "Water")
	virtual FVector GetSurfaceAt(const FVector& Location) const;

protected:
	UPROPERTY()
	class AEstPlayer* OverlappingPlayer;

	UPROPERTY()
	bool bPlayerImmersed;

	UPROPERTY()
	bool bPlayerPaddling;

	UPROPERTY()
	float LastPainTime;

	UPROPERTY()
	class UEstWaterManifest* LastManifest;
};
