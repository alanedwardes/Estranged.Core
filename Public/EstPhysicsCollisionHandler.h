#pragma once

#include "PhysicsEngine/PhysicsCollisionHandler.h"
#include "EstPhysicsCollisionHandler.generated.h"

UCLASS(abstract)
class ESTCORE_API UEstPhysicsCollisionHandler : public UPhysicsCollisionHandler
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class UEstImpactManifest* ImpactManifest;

	class UPhysicalMaterial* GetPhysicalMaterialFromCollision(FRigidBodyCollisionInfo CollisionInfo);

	virtual void HandlePhysicsCollisions_AssumesLocked(TArray<FCollisionNotifyInfo>& PendingCollisionNotifies) override;

	void CustomHandleCollision_AssumesLocked(const FRigidBodyCollisionInfo& MyInfo, const FRigidBodyCollisionInfo& OtherInfo, const FCollisionImpactData& RigidCollisionData);
};
