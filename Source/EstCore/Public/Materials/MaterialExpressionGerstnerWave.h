#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionGerstnerWave.generated.h"

class UEstWaterManifest;

/**
 * Calculates Gerstner Waves based on a set of waves defined in an EstWaterManifest.
 * 
 * Logic mirrors UEstWaterManifest::EvaluateWaveHeight on CPU.
 * Outputs:
 * - Offsets: World Position Offset (Vector3)
 * - Normal: Wave Surface Normal (Vector3)
 */
UCLASS(collapsecategories, hidecategories = Object, MinimalAPI)
class UMaterialExpressionGerstnerWave : public UMaterialExpression
{
	GENERATED_UCLASS_BODY()

	/** Reference to the Water Manifest containing wave data. */
	UPROPERTY(EditAnywhere, Category = "Gerstner Waves")
	TObjectPtr<UEstWaterManifest> WaterManifest;

	/** Absolute World Position input. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "If not connected, uses AbsoluteWorldPosition internal node."))
	FExpressionInput WorldPosition;

	/** Time input. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "If not connected, uses Time internal node."))
	FExpressionInput Time;

	/** Global scaler for wave amplitude. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Defaults to 1.0. Connect a ScalarParameter here to animate intensity."))
	FExpressionInput GlobalIntensity;

	//~ Begin UMaterialExpression Interface
#if WITH_EDITOR
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
	//~ End UMaterialExpression Interface
};
