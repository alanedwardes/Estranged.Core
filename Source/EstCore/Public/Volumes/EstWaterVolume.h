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

	virtual void SetPlayerPaddling(bool bEnabled);
	virtual void SetPlayerImmersed(bool bEnabled);

	virtual void ManifestChanged();

#if WITH_EDITORONLY_DATA
	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	bool bUpdateDebugOverlays;

	class USphereComponent* PainRadius;

	void OnSelectionChanged(UObject* NewSelection);

	void UpdateSelectionState();

	bool bSelectedInEditor;
#endif

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite, Interp)
	class UEstWaterManifest* Manifest;

	UFUNCTION(BlueprintPure, Category = "Water")
	virtual FVector GetSurface();

protected:
	virtual void CausePainTo(class AActor* Other);

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
