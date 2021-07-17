// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EstInstancedMeshArrayActor.generated.h"

UCLASS()
class ESTCORE_API AEstInstancedMeshArrayActor : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(Category = StaticMeshActor, VisibleAnywhere, BlueprintReadOnly)
	class UEstInstancedMeshArrayComponent* InstancedMeshArrayComponent;
};
