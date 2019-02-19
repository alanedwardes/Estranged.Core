// Estranged is a trade mark of Alan Edwardes.

#include "EstSkinnedMeshCacheComponent.h"
#include "EstCore.h"
#include "Runtime/Engine/Classes/Components/SkinnedMeshComponent.h"

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
		FEstSkinnedMeshComponentState SkinnedMeshState;
		SkinnedMeshState.ComponentName = SkinnedMeshComponent->GetFName();
		SkinnedMeshState.bIsSimulatingPhysics = SkinnedMeshComponent->IsSimulatingPhysics();

		for (int32 i = 0; i < SkinnedMeshComponent->GetNumBones(); i++)
		{
			const FName BoneName = SkinnedMeshComponent->GetBoneName(i);

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

		SkinnedMeshStates.Add(SkinnedMeshState);
	}
}

void UEstSkinnedMeshCacheComponent::RestoreSkinnedMeshStates()
{
	TArray<USkinnedMeshComponent*> SkinnedMeshComponents = GetSkinnedMeshComponents();

	for (const FEstSkinnedMeshComponentState State : SkinnedMeshStates)
	{
		USkinnedMeshComponent* SkinnedMeshComponent = GetSkinnedMeshComponentByName(State.ComponentName);

		for (const FEstSkinnedMeshBoneState BoneState : State.BoneStates)
		{
			FBodyInstance* BodyInstance = SkinnedMeshComponent->GetBodyInstance(BoneState.BoneName);
			BodyInstance->SetBodyTransform(BoneState.BoneTransform, ETeleportType::ResetPhysics);
			BodyInstance->SetInstanceSimulatePhysics(BoneState.bIsSimulatingPhysics);
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