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

	UPROPERTY()
	TObjectPtr<class UBoxComponent> Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Checkpoint)
	FVector TriggerExtent;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

#if	WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditUndo() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void Reposition();
#endif	// WITH_EDITOR
};
