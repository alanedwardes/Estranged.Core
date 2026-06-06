#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "EstCarryHandleComponent.generated.h"

UCLASS(ClassGroup=Physics, meta=(BlueprintSpawnableComponent))
class ESTCORE_API UEstCarryHandleComponent : public UPhysicsHandleComponent
{
	GENERATED_BODY()

public:
	UEstCarryHandleComponent();

	virtual void GrabComponentImp(UPrimitiveComponent* Component, FName InBoneName, const FVector& Location, const FRotator& Rotation, bool bInRotationConstrained) override;

	void ReleaseWithVelocity(FVector LinearVelocity, FVector AngularVelocity, float MaxLinearSpeed);
};
