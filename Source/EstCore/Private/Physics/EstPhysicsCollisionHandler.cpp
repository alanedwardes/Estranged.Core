// Estranged is a trade mark of Alan Edwardes.

#include "Physics/EstPhysicsCollisionHandler.h"
#include "EstCore.h"
#include "Physics/EstImpactManifest.h"
#include "PhysicsPublic.h"
#include "Runtime/Engine/Classes/Engine/TextureStreamingTypes.h"
#include "Runtime/Engine/Classes/Materials/MaterialInterface.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodyInstance.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/Components/PrimitiveComponent.h"
#include "Runtime/Engine/Classes/GameFramework/PhysicsVolume.h"
#include "Runtime/Engine/Public/PhysicsPublic.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Gameplay/EstGameInstance.h"
#include "Kismet/KismetMathLibrary.h"

void UEstPhysicsCollisionHandler::HandlePhysicsCollisions_AssumesLocked(TArray<FCollisionNotifyInfo>& PendingCollisionNotifies)
{
	// Fire any collision notifies in the queue.
	for (int32 i = 0; i<PendingCollisionNotifies.Num(); i++)
	{
		// If it hasn't been long enough since our last sound, just bail out
		const float TimeSinceLastImpact = GetWorld()->GetTimeSeconds() - LastImpactSoundTime;
		if (TimeSinceLastImpact < ImpactReFireDelay)
		{
			break;
		}

		// See if this impact is between 2 valid actors
		const FCollisionNotifyInfo& NotifyInfo = PendingCollisionNotifies[i];
		if (NotifyInfo.IsValidForNotify() &&
			NotifyInfo.RigidCollisionData.ContactInfos.Num() > 0)
		{
			CustomHandleCollision_AssumesLocked(NotifyInfo.Info0, NotifyInfo.Info1, NotifyInfo.RigidCollisionData);
		}
	}
}

void UEstPhysicsCollisionHandler::HandlePhysicsCollisions_AssumesLocked(const FChaosPhysicsCollisionInfo& CollisionInfo)
{
	const FVector RelVel = CollisionInfo.Velocity - CollisionInfo.OtherVelocity;

	// Then project along contact normal, and take magnitude.
	float ImpactVelMag = FMath::Abs(RelVel | CollisionInfo.Normal);

	DeployImpactEffect(ImpactManifest, CollisionInfo.Component, CollisionInfo.Location, CollisionInfo.Normal, ImpactVelMag);
	DeployImpactEffect(ImpactManifest, CollisionInfo.OtherComponent, CollisionInfo.Location, CollisionInfo.Normal, ImpactVelMag);
}

void UEstPhysicsCollisionHandler::HandlePhysicsBreak_AssumesLocked(const FChaosBreakEvent& BreakEvent)
{
	DeployImpactEffect(FractureManifest, BreakEvent.Component, BreakEvent.Location, FVector(), BreakEvent.Velocity.SquaredLength());
}

UPhysicalMaterial* UEstPhysicsCollisionHandler::GetPhysicalMaterialFromComponent(TWeakObjectPtr<UPrimitiveComponent> Component)
{
	if (!Component.IsValid())
	{
		return nullptr;
	}

	// For now, just the first material
	const UMaterialInterface* Material = Component->GetMaterial(0);
	if (Material == nullptr)
	{
		return nullptr;
	}

	return Material->GetPhysicalMaterial();
}

void UEstPhysicsCollisionHandler::CustomHandleCollision_AssumesLocked(const FRigidBodyCollisionInfo& MyInfo, const FRigidBodyCollisionInfo& OtherInfo, const FCollisionImpactData& RigidCollisionData)
{
	const FRigidBodyContactInfo ContactInfo = RigidCollisionData.ContactInfos[0];

	const FBodyInstance* BodyInst0 = MyInfo.GetBodyInstance();
	const FBodyInstance* BodyInst1 = OtherInfo.GetBodyInstance();

	if (BodyInst0 && BodyInst1)
	{
		// Find relative velocity.
		const FVector Velocity0 = BodyInst0->GetUnrealWorldVelocityAtPoint_AssumesLocked(ContactInfo.ContactPosition);
		const FVector AngularVel0 = BodyInst0->GetUnrealWorldAngularVelocityInRadians_AssumesLocked();

		const FVector Velocity1 = BodyInst1->GetUnrealWorldVelocityAtPoint_AssumesLocked(ContactInfo.ContactPosition);
		const FVector AngularVel1 = BodyInst1->GetUnrealWorldAngularVelocityInRadians_AssumesLocked();

		const FVector RelVel = Velocity1 - Velocity0;

		// Then project along contact normal, and take magnitude.
		float ImpactVelMag = FMath::Abs(RelVel | ContactInfo.ContactNormal);

		DeployImpactEffect(ImpactManifest, MyInfo.Component, ContactInfo.ContactPosition, ContactInfo.ContactNormal, ImpactVelMag);
		DeployImpactEffect(ImpactManifest, OtherInfo.Component, ContactInfo.ContactPosition, ContactInfo.ContactNormal, ImpactVelMag);

		UWorld* World = GetWorld();
		if (World == nullptr)
		{
			return;
		}

		LastImpactSoundTime = World->GetTimeSeconds();
	}
}

void UEstPhysicsCollisionHandler::DeployImpactEffect(const UEstImpactManifest* Manifest, TWeakObjectPtr<UPrimitiveComponent> Component, FVector Location, FVector Normal, float Velocity)
{
	if (Velocity < ImpactThreshold)
	{
		return;
	}

	const float Intensity = FMath::Clamp(Velocity / 512.f, 0.1f, 1.f);

	UPhysicalMaterial* OtherMaterial = GetPhysicalMaterialFromComponent(Component);
	if (OtherMaterial != nullptr && Component.IsValid() && Component->Mobility == EComponentMobility::Movable && !Component->GetOwner()->ActorHasTag(TAG_NOIMPACTS))
	{
		FEstImpactEffect ImpactEffect = UEstGameplayStatics::FindImpactEffect(Manifest, OtherMaterial);
		if (ImpactEffect != FEstImpactEffect::None)
		{
			UEstGameplayStatics::DeployImpactEffect(ImpactEffect, Location, Normal, Component.Get(), Intensity);
		}
	}
}
