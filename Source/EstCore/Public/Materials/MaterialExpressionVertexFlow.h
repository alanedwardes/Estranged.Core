#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionVertexFlow.generated.h"

/**
 * Calculates continuous flow UVs based on flow direction input or Mesh Vertex Colors.
 * 
 * Outputs:
 * - UV_A (Vector2): Distorted UVs for Phase 0.
 * - UV_B (Vector2): Distorted UVs for Phase 1.
 * - Alpha (Scalar): Pulse gradient (Triangle Wave) to blend between A and B.
 */
UCLASS(collapsecategories, hidecategories = Object, MinimalAPI)
class UMaterialExpressionVertexFlow : public UMaterialExpression
{
	GENERATED_UCLASS_BODY()

	/** Base UV coordinates to be distorted. Defaults to TextureCoorindates[0]. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Base UVs to distort."))
	FExpressionInput BaseUV;

	/** Flow direction input (RG = XY world direction, B = strength). 
	 *  Connect a MeshPaintTextureObject or flow map texture sample here. */
	UPROPERTY(meta = (RequiredInput = "true", ToolTip = "Flow direction (RG = world XY direction, B = strength)."))
	FExpressionInput FlowDirection;

	/** Global flow speed multiplier. Defaults to 1.0. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Speed of the flow cycle."))
	FExpressionInput FlowSpeed;

	/** Global distortion strength. Defaults to 0.2. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Strength of the UV distortion."))
	FExpressionInput FlowStrength;

	/** Optional Phase Offset (0-1). Connect a Noise Mask here to randomize the blend timing per-pixel. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Randomize loop timing per pixel."))
	FExpressionInput PhaseOffset;

	/** Time input override. Defaults to Game Time. */
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Override time for the animation."))
	FExpressionInput Time;

	//~ Begin UMaterialExpression Interface
#if WITH_EDITOR
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
	//~ End UMaterialExpression Interface
};
