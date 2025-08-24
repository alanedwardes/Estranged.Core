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

	/**
	 * Controls how strongly this mesh floats or sinks in water.
	 *
	 *   BuoyancyCoefficient Value Table:
	 *   ---------------------------------------------------------------
	 *   Value                   | Effect/Interpretation
	 *   ---------------------------------------------------------------
	 *   0.0                     | No buoyancy (object sinks)
	 *   >0.0 and <1.0           | Reduced buoyancy (object sinks or barely floats)
	 *   1.0                     | Neutral buoyancy (matches water, floats at surface)
	 *   >1.0                    | Strong buoyancy (floats high, rises quickly)
	 *   Negative values         | Physically incorrect (should not be used)
	 *   ---------------------------------------------------------------
	 *
	 *   Examples:
	 *     - Cork: BuoyancyCoefficient = 1.5 (floats high)
	 *     - Wood: BuoyancyCoefficient = 1.1 (floats well)
	 *     - Plastic: BuoyancyCoefficient = 1.0 (floats at surface)
	 *     - Iron: BuoyancyCoefficient = 0.1 (sinks)
	 *     - Lead: BuoyancyCoefficient = 0.0 (sinks, no buoyancy)
	 */
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
