#pragma once

#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "PhysicsEngine/PhysicsCollisionHandler.h"
#include "EstPhysicsCollisionHandler.generated.h"

UCLASS(abstract)
class ESTCORE_API UEstPhysicsCollisionHandler : public UPhysicsCollisionHandler
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class UEstImpactManifest* ImpactManifest;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class UEstImpactManifest* FractureManifest;

	// The time to wait after the world is brought up before impact effects can start to happen
	// Prevents physics objects which are settling from being heard right at the start of play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Impact)
	float WorldUpDelay;

	class UPhysicalMaterial* GetPhysicalMaterialFromComponent(TWeakObjectPtr<UPrimitiveComponent> Component);

	virtual void HandlePhysicsCollisions_AssumesLocked(TArray<FCollisionNotifyInfo>& PendingCollisionNotifies) override;

	virtual void HandlePhysicsCollisions_AssumesLocked(const FChaosPhysicsCollisionInfo& CollisionInfo);

	virtual void HandlePhysicsBreak_AssumesLocked(const FChaosBreakEvent& BreakEvent);

	void CustomHandleCollision_AssumesLocked(const FRigidBodyCollisionInfo& MyInfo, const FRigidBodyCollisionInfo& OtherInfo, const FCollisionImpactData& RigidCollisionData);
	void DeployImpactEffect(const UEstImpactManifest* Manifest, TWeakObjectPtr<UPrimitiveComponent> Component, FVector Location, FVector Normal, float Velocity);
};
