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
	/** Mass override in kilograms - zero means no effect */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Units = "kg"))
	float Mass;

	/** Buoyancy coefficient or upward force multiplier - zero means no effect */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ClampMin = "0.0"))
	float BuoyancyCoefficient = 0.f;

	/**
	* If greater than zero, the number of degrees the object must be tilted
	* to trigger self-righting. For example, a value of 20 means the object
	* will self-right if tilted more than 20 degrees from upright.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Units = "deg", ClampMin = "0.0", ClampMax = "360.0"))
	float SelfRightingDegrees = 0.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Units = "cm"))
	float HullOffset;
};
