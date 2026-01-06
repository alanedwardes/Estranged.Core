#include "Materials/MaterialExpressionGerstnerWave.h"
#include "Volumes/EstWaterManifest.h"
#include "MaterialCompiler.h"

UMaterialExpressionGerstnerWave::UMaterialExpressionGerstnerWave(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	MenuCategories.Add(FText::FromString(TEXT("Estranged")));
#endif
}

#if WITH_EDITOR

int32 UMaterialExpressionGerstnerWave::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	// 1. Compile Inputs
	int32 WorldPosIndex = WorldPosition.GetTracedInput().Expression 
		? WorldPosition.Compile(Compiler) 
		: Compiler->WorldPosition(EWorldPositionIncludedOffsets::WPT_Default);

	int32 TimeIndex = Time.GetTracedInput().Expression 
		? Time.Compile(Compiler) 
		: Compiler->GameTime(false, 0.0f); 

	int32 IntensityIndex = GlobalIntensity.GetTracedInput().Expression
		? GlobalIntensity.Compile(Compiler)
		: Compiler->Constant(1.0f);

	if (!WaterManifest)
	{
		Compiler->Error(TEXT("Missing Water Manifest"));
		return INDEX_NONE;
	}

	// 2. Build Math Graph
	// Logic must STRICTLY match UEstWaterManifest::EvaluateWaveOffsets
	// We bake the constants from the Manifest directly into the shader instructions.
	
	int32 OffsetsX = Compiler->Constant(0.0f);
	int32 OffsetsY = Compiler->Constant(0.0f);
	int32 OffsetsZ = Compiler->Constant(0.0f);

	int32 PosX = Compiler->ComponentMask(WorldPosIndex, true, false, false, false);
	int32 PosY = Compiler->ComponentMask(WorldPosIndex, false, true, false, false);

	for (int32 i = 0; i < WaterManifest->Waves.Num(); ++i)
	{
		const FEstGerstnerWave& Wave = WaterManifest->Waves[i];
		if (Wave.Wavelength <= KINDA_SMALL_NUMBER) continue;

		// Calculate constants on CPU
		const float PI_VAL = 3.1415926535f; 
		float K = 2.0f * PI_VAL / Wave.Wavelength;
		float C = FMath::Sqrt(980.0f / K);

		// Material Nodes for constants
		int32 NodeDirX = Compiler->Constant(Wave.Direction.X);
		int32 NodeDirY = Compiler->Constant(Wave.Direction.Y);
		int32 NodeK = Compiler->Constant(K);
		int32 NodeC = Compiler->Constant(C);
		
		// Apply Global Intensity to Amplitude
		int32 NodeAmp = Compiler->Mul(Compiler->Constant(Wave.Amplitude), IntensityIndex);
		
		int32 NodeSteep = Compiler->Constant(Wave.Steepness);

		// float DotP = (Dir.X * Pos2D.X) + (Dir.Y * Pos2D.Y);
		int32 DotP = Compiler->Add(
			Compiler->Mul(NodeDirX, PosX),
			Compiler->Mul(NodeDirY, PosY)
		);

		// float Phase = K * DotP - (C * K * Time);
		// CK = C * K
		int32 NodeCK = Compiler->Mul(NodeC, NodeK);
		
		int32 Phase = Compiler->Sub(
			Compiler->Mul(NodeK, DotP),
			Compiler->Mul(NodeCK, TimeIndex)
		);

		// SinP, CosP
		int32 SinP = Compiler->Sine(Phase);
		int32 CosP = Compiler->Cosine(Phase);

		// Z Accumulation (Cos)
		// Offsets.z += Amplitude * CosP;
		OffsetsZ = Compiler->Add(OffsetsZ, Compiler->Mul(NodeAmp, CosP));

		// XY Accumulation
		// WA = Steepness * Amplitude
		// Offsets.x += WA * DirX * SinP;
		// Offsets.y += WA * DirY * SinP;
		int32 WA = Compiler->Mul(NodeSteep, NodeAmp);
		int32 CommonTerm = Compiler->Mul(WA, SinP);

		OffsetsX = Compiler->Add(OffsetsX, Compiler->Mul(CommonTerm, NodeDirX));
		OffsetsY = Compiler->Add(OffsetsY, Compiler->Mul(CommonTerm, NodeDirY));
	}

	int32 Result = Compiler->AppendVector(
		Compiler->AppendVector(OffsetsX, OffsetsY),
		OffsetsZ
	);

	return Result;
}

void UMaterialExpressionGerstnerWave::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("Estranged Gerstner Waves"));
}

#endif

#undef LOCTEXT_NAMESPACE
