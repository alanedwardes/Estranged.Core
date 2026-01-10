// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "EstPainSphereComponent.generated.h"

/**
 * A sphere which causes pain to actors overlapping it (or outside of it, if inverted).
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class ESTCORE_API UEstPainSphereComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UEstPainSphereComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** If true, pain is caused OUTSIDE the sphere instead of INSIDE. */
	UPROPERTY(Category = "Pain", EditAnywhere, BlueprintReadWrite)
	bool bInverted;

	/** Damage done per second to actors. */
	UPROPERTY(Category = "Pain", EditAnywhere, BlueprintReadWrite)
	float DamagePerSec;

	/** Time between damage applications. */
	UPROPERTY(Category = "Pain", EditAnywhere, BlueprintReadWrite)
	float PainInterval;

	/** Type of damage done. */
	UPROPERTY(Category = "Pain", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageType> DamageType;

protected:
	void CausePainTo(AActor* Other);

	UFUNCTION()
	void OnPainComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPainComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> ActorsToSuffer;

	UPROPERTY()
	float LastPainTime;
};
