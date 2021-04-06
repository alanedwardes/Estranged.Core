// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "EstInstancedMeshArray.generated.h"

UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent), Blueprintable)
class ESTCORE_API UEstInstancedMeshArray : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform ArrayInstanceOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ArrayInstanceCount;

	virtual void RebuildInstances();

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
