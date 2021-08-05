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

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Push;

private:
	UPROPERTY()
	TSet<APawn*> Pawns;
};
