// Estranged is a trade mark of Alan Edwardes.

#include "EstCore.h"
#include "EstSkinnedMeshCacheComponent.h"

void UEstSkinnedMeshCacheComponent::OnPostRestore_Implementation()
{
	TArray<USkinnedMeshComponent*> SkinnedMeshComponents = GetSkinnedMeshComponents();

	for (const FEstSkinnedMeshComponentState State : SkinnedMeshStates)
	{
		USkinnedMeshComponent* SkinnedMeshComponent = GetSkinnedMeshComponentByName(State.ComponentName);
		SkinnedMeshComponent->SetSimulatePhysics(State.bIsSimulatingPhysics);

		for (const FEstSkinnedMeshBoneState BoneState : State.BoneStates)
		{
			FBodyInstance* BodyInstance = SkinnedMeshComponent->GetBodyInstance(BoneState.BoneName);
			BodyInstance->SetBodyTransform(BoneState.BoneTransform, ETeleportType::ResetPhysics);
			BodyInstance->SetInstanceSimulatePhysics(BoneState.bIsSimulatingPhysics);
		}
	}
}

void UEstSkinnedMeshCacheComponent::OnPreSave_Implementation()
{
	SkinnedMeshStates.Empty();

	for (const USkinnedMeshComponent* SkinnedMeshComponent : GetSkinnedMeshComponents())
	{
		FEstSkinnedMeshComponentState SkinnedMeshState;
		SkinnedMeshState.ComponentName = SkinnedMeshComponent->GetFName();
		SkinnedMeshState.bIsSimulatingPhysics = SkinnedMeshComponent->IsSimulatingPhysics();

		for (int32 i = 0; i < SkinnedMeshComponent->GetNumBones(); i++)
		{
			FName BoneName = SkinnedMeshComponent->GetBoneName(i);
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

		SkinnedMeshStates.Add(SkinnedMeshState);
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
	return *GetSkinnedMeshComponents().FindByPredicate([&](const USkinnedMeshComponent* SkinnedMeshComponent) { return SkinnedMeshComponent->GetFName() == ComponentName; });
}