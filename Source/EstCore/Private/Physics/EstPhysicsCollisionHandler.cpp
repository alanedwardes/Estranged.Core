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
#include "PhysicalMaterials/PhysicalMaterial.h"

#define MAX_VELOCITY 512.f

UEstPhysicsCollisionHandler::UEstPhysicsCollisionHandler(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WorldUpDelay = 2.5f;
}

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
	const float Velocity = BreakEvent.Velocity.IsNearlyZero() ? MAX_VELOCITY : BreakEvent.Velocity.SquaredLength();
	DeployImpactEffect(FractureManifest, BreakEvent.Component, BreakEvent.Location, FVector(), Velocity);
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

	if (GetWorld()->GetRealTimeSeconds() < WorldUpDelay)
	{
		UE_LOG(LogEstPhysicsImpacts, Warning, TEXT("Not deploying impact effect for %s because it impacted too soon after the world came up (currently %.2fs, need to wait until %.2fs)"), *Component->GetName(), GetWorld()->GetRealTimeSeconds(), WorldUpDelay);
		return;
	}

	if (!Component.IsValid())
	{
		UE_LOG(LogEstPhysicsImpacts, Warning, TEXT("Can't deploy impact effect as component is not a live UObject"));
		return;
	}

	if (Component->Mobility != EComponentMobility::Movable)
	{
		UE_LOG(LogEstPhysicsImpacts, Warning, TEXT("Not deploying impact effect for %s because it is not movable"), *Component->GetName());
		return;
	}

	if (Component->GetOwner()->ActorHasTag(TAG_NOIMPACTS))
	{
		UE_LOG(LogEstPhysicsImpacts, Warning, TEXT("Not deploying impact effect for %s because it has the tag NOIMPACTS"), *Component->GetName());
		return;
	}

	UPhysicalMaterial* PhysicalMaterial = GetPhysicalMaterialFromComponent(Component);
	if (PhysicalMaterial == nullptr)
	{
		UE_LOG(LogEstPhysicsImpacts, Warning, TEXT("Not deploying impact effect for %s because it doesn't have a physical material"), *Component->GetName());
		return;
	}

	FEstImpactEffect ImpactEffect = UEstGameplayStatics::FindImpactEffect(Manifest, PhysicalMaterial);
	if (ImpactEffect == FEstImpactEffect::None)
	{
		UE_LOG(LogEstPhysicsImpacts, Warning, TEXT("Not deploying impact effect for %s because its physical material %s has no effects in the manifest %s"), *Component->GetName(), *PhysicalMaterial->GetName(), *Manifest->GetName());
		return;
	}

	const float Intensity = FMath::Clamp(Velocity / MAX_VELOCITY, 0.1f, 1.f);
	UEstGameplayStatics::DeployImpactEffect(ImpactEffect, Location, Normal, Component.Get(), Intensity);
}
