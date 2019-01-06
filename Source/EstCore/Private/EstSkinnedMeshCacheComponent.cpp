// Estranged is a trade mark of Alan Edwardes.

#include "EstSkinnedMeshCacheComponent.h"
#include "EstCore.h"
#include "DestructibleComponent.h"
#include "PhysXPublic.h"

UEstSkinnedMeshCacheComponent::UEstSkinnedMeshCacheComponent(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoRestore = true;
}

void UEstSkinnedMeshCacheComponent::OnPostRestore_Implementation()
{
	if (bAutoRestore)
	{
		RestoreSkinnedMeshStates();
	}
}

void UEstSkinnedMeshCacheComponent::OnPreSave_Implementation()
{
	SkinnedMeshStates.Empty();

	for (const USkinnedMeshComponent* SkinnedMeshComponent : GetSkinnedMeshComponents())
	{
		const UDestructibleComponent* DestructibleComponent = Cast<UDestructibleComponent>(SkinnedMeshComponent);

		FEstSkinnedMeshComponentState SkinnedMeshState;
		SkinnedMeshState.ComponentName = SkinnedMeshComponent->GetFName();
		SkinnedMeshState.bIsSimulatingPhysics = SkinnedMeshComponent->IsSimulatingPhysics();

		for (int32 i = 0; i < SkinnedMeshComponent->GetNumBones(); i++)
		{
			const FName BoneName = SkinnedMeshComponent->GetBoneName(i);

			if (DestructibleComponent)
			{
				const int32 ChunkIndex = DestructibleComponent->BoneIdxToChunkIdx(i);
				const PxRigidDynamic* PhysXActor = DestructibleComponent->ApexDestructibleActor->getChunkPhysXActor(ChunkIndex);

				// If this is the root chunk, there is no physics actor, or the chunk is not kinematic, skip it
				if (BoneName == FName("Root") || !PhysXActor || PhysXActor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
				{
					continue;
				}

				FEstSkinnedMeshBoneState BoneState;
				BoneState.BoneName = BoneName;
				BoneState.BoneTransform = DestructibleComponent->GetBoneTransform(i);
				SkinnedMeshState.BoneStates.Add(BoneState);
			}
			else
			{
				// If there is not body for this bone, skip it
				if (SkinnedMeshComponent->GetBodyInstance(BoneName) == nullptr)
				{
					continue;
				}

				FEstSkinnedMeshBoneState BoneState;
				BoneState.BoneName = BoneName;
				BoneState.BoneTransform = SkinnedMeshComponent->GetBoneTransform(i);
				BoneState.bIsSimulatingPhysics = SkinnedMeshComponent->IsSimulatingPhysics(BoneName);
				SkinnedMeshState.BoneStates.Add(BoneState);
			}
		}

		SkinnedMeshStates.Add(SkinnedMeshState);
	}
}

void UEstSkinnedMeshCacheComponent::RestoreSkinnedMeshStates()
{
	TArray<USkinnedMeshComponent*> SkinnedMeshComponents = GetSkinnedMeshComponents();

	for (const FEstSkinnedMeshComponentState State : SkinnedMeshStates)
	{
		USkinnedMeshComponent* SkinnedMeshComponent = GetSkinnedMeshComponentByName(State.ComponentName);

		UDestructibleComponent* DestructibleComponent = Cast<UDestructibleComponent>(SkinnedMeshComponent);

		for (const FEstSkinnedMeshBoneState BoneState : State.BoneStates)
		{
			if (DestructibleComponent)
			{
				const int32 BoneIndex = DestructibleComponent->GetBoneIndex(BoneState.BoneName);
				const int32 ChunkIndex = DestructibleComponent->BoneIdxToChunkIdx(BoneIndex);
				DestructibleComponent->ApexDestructibleActor->setDynamic(ChunkIndex);
				// TODO: Set chunk location?
			}
			else
			{
				FBodyInstance* BodyInstance = SkinnedMeshComponent->GetBodyInstance(BoneState.BoneName);
				BodyInstance->SetBodyTransform(BoneState.BoneTransform, ETeleportType::ResetPhysics);
				BodyInstance->SetInstanceSimulatePhysics(BoneState.bIsSimulatingPhysics);
			}
		}
	}
}

TArray<USkinnedMeshComponent*> UEstSkinnedMeshCacheComponent::GetSkinnedMeshComponents()
{
	TArray<USkinnedMeshComponent*> SkinnedMeshComponents;
	GetOwner()->GetComponents<USkinnedMeshComponent>(SkinnedMeshComponents);
	return SkinnedMeshComponents;
}

USkinnedMeshComponent* UEstSkinnedMeshCacheComponent::GetSkinnedMeshComponentByName(FName ComponentName)
{
	return *GetSkinnedMeshComponents().FindByPredicate([&](const USkinnedMeshComponent* Component) { return Component->GetFName() == ComponentName; });
}