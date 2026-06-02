#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionPerspectiveSpin.generated.h"

UCLASS(collapsecategories, hidecategories = Object, MinimalAPI)
class UMaterialExpressionPerspectiveSpin : public UMaterialExpression
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Input UVs. Defaults to TexCoord[0]."))
	FExpressionInput UVs;

	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Rotation angle in radians around the Y axis. Defaults to 0."))
	FExpressionInput Angle;

	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Perspective focal distance. Higher = flatter. Defaults to 2.0."))
	FExpressionInput FocalDistance;

	UPROPERTY(EditAnywhere, Category = "Perspective Spin")
	bool bDoubleSided;

	UPROPERTY(EditAnywhere, Category = "Perspective Spin", meta = (EditCondition = "bDoubleSided"))
	bool bFlipBackFace;

#if WITH_EDITOR
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif
};
