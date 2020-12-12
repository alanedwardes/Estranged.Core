#pragma once

#include "Runtime/CoreUObject/Public/UObject/Interface.h"
#include "EstPreBeginPlay.generated.h"

UINTERFACE()
class ESTCORE_API UEstPreBeginPlay : public UInterface
{
	GENERATED_BODY()
};

class ESTCORE_API IEstPreBeginPlay
{
	GENERATED_IINTERFACE_BODY()

	UFUNCTION(BlueprintNativeEvent)
	void OnPreBeginPlay();
};
