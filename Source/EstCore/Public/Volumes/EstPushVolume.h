// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "EstPushVolume.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API AEstPushVolume : public AVolume
{
	GENERATED_UCLASS_BODY()

public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp)
	FVector Push;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Interp)
	float Intensity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSet<APawn*> Pawns;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSet<AActor*> Actors;
};
