// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "GameFramework/Volume.h"
#include "EstFootstepVolume.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API AEstFootstepVolume : public AVolume
{
	GENERATED_UCLASS_BODY()
	
public:
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite)
	class UPhysicalMaterial* FootstepMaterialOverride;

	void NotifyActorBeginOverlap(AActor* OtherActor) override;

	void NotifyActorEndOverlap(AActor* OtherActor) override;
};
