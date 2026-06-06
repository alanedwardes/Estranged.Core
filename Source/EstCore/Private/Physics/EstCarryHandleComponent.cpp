#include "Physics/EstCarryHandleComponent.h"
#include "Components/PrimitiveComponent.h"

UEstCarryHandleComponent::UEstCarryHandleComponent()
{
	LinearStiffness = 1500.f;
	LinearDamping = 200.f;
	AngularStiffness = 3000.f;
	AngularDamping = 500.f;
}

void UEstCarryHandleComponent::GrabComponentImp(UPrimitiveComponent* Component, FName InBoneName, const FVector& Location, const FRotator& Rotation, bool bInRotationConstrained)
{
	if (!Component)
	{
		return;
	}

	Component->SetEnableGravity(false);
	Component->SetAllUseCCD(true);

	Super::GrabComponentImp(Component, InBoneName, Location, Rotation, bInRotationConstrained);
}

void UEstCarryHandleComponent::ReleaseWithVelocity(FVector LinearVelocity, FVector AngularVelocity, float MaxLinearSpeed)
{
	UPrimitiveComponent* Grabbed = GetGrabbedComponent();
	if (!Grabbed)
	{
		return;
	}

	ReleaseComponent();

	Grabbed->SetEnableGravity(true);

	Grabbed->SetPhysicsLinearVelocity(LinearVelocity.GetClampedToMaxSize(MaxLinearSpeed));
	Grabbed->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
}
