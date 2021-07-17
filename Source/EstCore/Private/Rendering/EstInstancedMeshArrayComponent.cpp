#include "Rendering/EstInstancedMeshArrayComponent.h"

void UEstInstancedMeshArrayComponent::RebuildInstances()
{
	ClearInstances();

	FTransform LastTransform;

	TArray<FTransform> InstanceTransforms;

	for (int i = 0; i < ArrayInstanceCount; i++)
	{
		InstanceTransforms.Add(LastTransform);
		LastTransform *= ArrayInstanceOffset;
	}

	AddInstances(InstanceTransforms, false);
}

#if WITH_EDITOR
void UEstInstancedMeshArrayComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	RebuildInstances();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif