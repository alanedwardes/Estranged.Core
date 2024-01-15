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
	virtual void ActorEnteredVolume(class AActor* Other) override;
	virtual void ActorLeavingVolume(class AActor* Other) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	virtual void Tick(float DeltaTime) override;

	virtual void SetPlayerImmersed(bool bEnabled);
	virtual void SetPlayerPaddling(bool bEnabled);

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UEstWaterManifest* Manifest;

	UFUNCTION(BlueprintPure, Category = "Water")
	virtual FVector GetSurface();

protected:
	virtual void CausePainTo(class AActor* Other);
	virtual void PainTimer();

	UPROPERTY()
	class AEstPlayer* OverlappingPlayer;

	UPROPERTY()
	bool bPlayerImmersed;

	UPROPERTY()
	bool bPlayerPaddling;

	UPROPERTY()
	FTimerHandle TimerHandle_PainTimer;
};
