// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "EstPhysicsUserData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew)
class ESTCORE_API UEstPhysicsUserData : public UAssetUserData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Mass;
};
