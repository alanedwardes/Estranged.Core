#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionGerstnerWave.generated.h"


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

	/** Absolute World Position input. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "If not connected, uses AbsoluteWorldPosition internal node."))
	FExpressionInput WorldPosition;

	/** Time input. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "If not connected, uses Time internal node."))
	FExpressionInput Time;

	/** Global scaler for wave amplitude. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Defaults to 1.0. Connect a ScalarParameter here to animate intensity."))
	FExpressionInput GlobalIntensity;

	/** Excluder input (Vector4: X, Y, Radius, FadeRadius). */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Pack X, Y, Radius, FadeRadius into a Vector4 to exclude waves."))
	FExpressionInput WaveExcluder;

	/** Packed Wave 1 (Vector4: Angle, Wavelength, Amplitude, Steepness). */
	UPROPERTY(meta = (RequiredInput = "false"))
	FExpressionInput Wave1;

	/** Packed Wave 2 (Vector4: Angle, Wavelength, Amplitude, Steepness). */
	UPROPERTY(meta = (RequiredInput = "false"))
	FExpressionInput Wave2;

	/** Packed Wave 3 (Vector4: Angle, Wavelength, Amplitude, Steepness). */
	UPROPERTY(meta = (RequiredInput = "false"))
	FExpressionInput Wave3;

	/** Packed Wave 4 (Vector4: Angle, Wavelength, Amplitude, Steepness). */
	UPROPERTY(meta = (RequiredInput = "false"))
	FExpressionInput Wave4;

	/** Packed Wave 5 (Vector4: Angle, Wavelength, Amplitude, Steepness). */
	UPROPERTY(meta = (RequiredInput = "false"))
	FExpressionInput Wave5;

	/** Packed Wave 6 (Vector4: Angle, Wavelength, Amplitude, Steepness). */
	UPROPERTY(meta = (RequiredInput = "false"))
	FExpressionInput Wave6;

	/** Packed Wave 7 (Vector4: Angle, Wavelength, Amplitude, Steepness). */
	UPROPERTY(meta = (RequiredInput = "false"))
	FExpressionInput Wave7;

	/** Packed Wave 8 (Vector4: Angle, Wavelength, Amplitude, Steepness). */
	UPROPERTY(meta = (RequiredInput = "false"))
	FExpressionInput Wave8;

	//~ Begin UMaterialExpression Interface
#if WITH_EDITOR
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
	//~ End UMaterialExpression Interface
};
