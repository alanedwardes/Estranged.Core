// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/EstPlayerStart.h"
#include "EstPlayerCheckpoint.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API AEstPlayerCheckpoint : public AEstPlayerStart
{
	GENERATED_BODY()

public:
	AEstPlayerCheckpoint(const class FObjectInitializer& PCIP);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Checkpoint)
	class UBoxComponent* Trigger;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
