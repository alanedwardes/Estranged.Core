// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "EstResetVolume.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API AEstResetVolume : public AVolume
{
	GENERATED_UCLASS_BODY()
	
public:
	void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY(Category = "Reset", EditAnywhere, BlueprintReadWrite)
	TSet<AActor*> TargetsSet;
};
