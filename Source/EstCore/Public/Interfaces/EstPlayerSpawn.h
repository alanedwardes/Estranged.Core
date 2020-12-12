#pragma once

#include "Runtime/CoreUObject/Public/UObject/Interface.h"
#include "EstPlayerSpawn.generated.h"

UINTERFACE()
class ESTCORE_API UEstPlayerSpawn : public UInterface
{
	GENERATED_BODY()
};

class ESTCORE_API IEstPlayerSpawn
{
	GENERATED_IINTERFACE_BODY()

	UFUNCTION(BlueprintNativeEvent)
	void OnPlayerSpawn(AController* NewPlayer, ACharacter* NewCharacter);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnPlayerSpawnInside(AController* NewPlayer, ACharacter* NewCharacter);
};
