// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "EstCarryableUserData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew)
class ESTCORE_API UEstCarryableUserData : public UAssetUserData
{
	GENERATED_BODY()

public:
	UEstCarryableUserData();

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bCanPlayerPickUp;
};
