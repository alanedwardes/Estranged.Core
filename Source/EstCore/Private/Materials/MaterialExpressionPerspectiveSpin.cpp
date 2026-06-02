// Inverse perspective projection for a flat card spinning around Y.
// Centre the UVs, compute the perspective denominator (F*cos - px*sin),
// then divide back out. Negative D means back face -- the divide mirrors tu for free.
// Double-sided mode clamps |D| and restores the sign to kill ghosting at grazing angles.

#include "Materials/MaterialExpressionPerspectiveSpin.h"
#include "MaterialCompiler.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(MaterialExpressionPerspectiveSpin)

UMaterialExpressionPerspectiveSpin::UMaterialExpressionPerspectiveSpin(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	MenuCategories.Add(FText::FromString(TEXT("Estranged")));
#endif

	bDoubleSided = false;
	bFlipBackFace = false;

	Outputs.Reset();
	Outputs.Add(FExpressionOutput(TEXT("Projected UVs")));
	Outputs.Add(FExpressionOutput(TEXT("Alpha")));

	bShowOutputNameOnPin = true;
}

#if WITH_EDITOR

int32 UMaterialExpressionPerspectiveSpin::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	int32 UVsIndex = UVs.GetTracedInput().Expression
		? UVs.Compile(Compiler)
		: Compiler->TextureCoordinate(0, false, false);

	int32 AngleIndex = Angle.GetTracedInput().Expression
		? Angle.Compile(Compiler)
		: Compiler->Constant(0.0f);

	int32 FocalIndex = FocalDistance.GetTracedInput().Expression
		? FocalDistance.Compile(Compiler)
		: Compiler->Constant(2.0f);

	int32 Half = Compiler->Constant(0.5f);
	int32 One  = Compiler->Constant(1.0f);
	int32 PX = Compiler->Sub(Compiler->ComponentMask(UVsIndex, true, false, false, false), Half);
	int32 PY = Compiler->Sub(Compiler->ComponentMask(UVsIndex, false, true, false, false), Half);

	int32 CosA = Compiler->Cosine(AngleIndex);
	int32 SinA = Compiler->Sine(AngleIndex);

	int32 DenomRaw = Compiler->Sub(
		Compiler->Mul(FocalIndex, CosA),
		Compiler->Mul(PX, SinA)
	);

	int32 DenomSafe;
	int32 IsFront = INDEX_NONE;
	if (bDoubleSided)
	{
		IsFront         = Compiler->Step(Compiler->Constant(0.0f), CosA);
		int32 SignDenom = Compiler->Sub(Compiler->Mul(Compiler->Constant(2.0f), IsFront), One);
		DenomSafe = Compiler->Mul(
			Compiler->Max(Compiler->Abs(DenomRaw), Compiler->Constant(1e-4f)),
			SignDenom
		);
	}
	else
	{
		DenomSafe = Compiler->Max(DenomRaw, Compiler->Constant(1e-4f));
	}

	int32 TU = Compiler->Add(Half, Compiler->Div(Compiler->Mul(PX, FocalIndex), DenomSafe));
	int32 TV = Compiler->Add(Half, Compiler->Div(Compiler->Mul(Compiler->Mul(PY, FocalIndex), CosA), DenomSafe));

	if (bDoubleSided && bFlipBackFace)
	{
		int32 TU_flipped = Compiler->Sub(One, TU);
		TU = Compiler->Add(
			Compiler->Mul(IsFront, TU),
			Compiler->Mul(Compiler->Sub(One, IsFront), TU_flipped)
		);
	}

	if (OutputIndex == 0)
	{
		return Compiler->AppendVector(TU, TV);
	}

	int32 FaceAlpha = bDoubleSided
		? Compiler->Step(Compiler->Constant(0.0f), Compiler->Mul(CosA, DenomRaw))
		: Compiler->Step(Compiler->Constant(0.0f), DenomRaw);

	int32 InU = Compiler->Mul(
		Compiler->Step(Compiler->Constant(0.0f), TU),
		Compiler->Step(TU, One)
	);
	int32 InV = Compiler->Mul(
		Compiler->Step(Compiler->Constant(0.0f), TV),
		Compiler->Step(TV, One)
	);
	return Compiler->Mul(FaceAlpha, Compiler->Mul(InU, InV));
}

void UMaterialExpressionPerspectiveSpin::GetCaption(TArray<FString>& OutCaptions) const
{
	if (!bDoubleSided)       OutCaptions.Add(TEXT("Estranged Perspective Spin"));
	else if (bFlipBackFace)  OutCaptions.Add(TEXT("Estranged Perspective Spin (Double Sided, Flip Back)"));
	else                     OutCaptions.Add(TEXT("Estranged Perspective Spin (Double Sided)"));
}

#endif
