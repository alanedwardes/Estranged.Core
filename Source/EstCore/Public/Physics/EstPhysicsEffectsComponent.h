// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "EstPhysicsEffectsComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEstPhysicsEffectsComponent, Log, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESTCORE_API UEstPhysicsEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEstPhysicsEffectsComponent();

protected:
	UFUNCTION()
	virtual void OnRegister() override;

	UFUNCTION()
	virtual void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void OnChaosPhysicsCollision(const FChaosPhysicsCollisionInfo& CollisionInfo);

	UFUNCTION()
	virtual void OnChaosBreak(const FChaosBreakEvent& BreakEvent);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

private:
	UPROPERTY()
	class APhysicsVolume* CurrentPhysicsVolume;

	UPROPERTY()
	float MaxBuoyancyCoefficient = 0.0f;
};
