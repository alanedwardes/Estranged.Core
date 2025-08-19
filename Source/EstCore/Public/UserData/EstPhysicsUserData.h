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
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float BuoyancyCoefficient = 0.f;
};
