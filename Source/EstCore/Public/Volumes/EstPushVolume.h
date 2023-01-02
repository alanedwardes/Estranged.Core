// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PhysicsVolume.h"
#include "EstPushVolume.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API AEstPushVolume : public APhysicsVolume
{
	GENERATED_UCLASS_BODY()

public:
	virtual void ActorEnteredVolume(class AActor* Other) override;

	virtual void ActorLeavingVolume(class AActor* Other) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp)
	FVector Push;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp)
	float Intensity;

private:
	UPROPERTY()
	TSet<APawn*> Pawns;
	UPROPERTY()
	TSet<AActor*> Actors;
};
