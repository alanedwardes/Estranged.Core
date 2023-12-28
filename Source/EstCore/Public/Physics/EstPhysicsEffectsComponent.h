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
	virtual void OnChaosPhysicsCollision(const FChaosPhysicsCollisionInfo& CollisionInfo);

	UFUNCTION()
	virtual void OnChaosBreak(const FChaosBreakEvent& BreakEvent);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
