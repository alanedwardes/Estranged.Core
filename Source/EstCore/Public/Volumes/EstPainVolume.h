// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "EstPainVolume.generated.h"

/**
 * A volume which causes pain to actors overlapping it.
 * Alternative to PainCausingVolume which is not a PhysicsVolume.
 */
UCLASS()
class ESTCORE_API AEstPainVolume : public AVolume
{
	GENERATED_BODY()

public:
	AEstPainVolume();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	/** Whether damage is currently applied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPainCausing;

	/** Damage done per second to actors. */
	UPROPERTY(Category = "Pain", EditAnywhere, BlueprintReadWrite)
	float DamagePerSec;

	/** Time between damage applications. */
	UPROPERTY(Category = "Pain", EditAnywhere, BlueprintReadWrite)
	float PainInterval;

	/** Whether to apply damage immediately on entry. */
	UPROPERTY(Category = "Pain", EditAnywhere, BlueprintReadWrite)
	bool bEntryPain;

	/** Type of damage done. */
	UPROPERTY(Category = "Pain", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageType> DamageType;

protected:
	void CausePainTo(AActor* Other);
	void PainTimer();

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> ActorsToSuffer;

	UPROPERTY(Transient)
	FTimerHandle PainTimerHandle;
};
