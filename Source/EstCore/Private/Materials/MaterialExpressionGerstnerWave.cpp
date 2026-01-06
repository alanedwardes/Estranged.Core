#include "Materials/MaterialExpressionGerstnerWave.h"
#include "Volumes/EstWaterManifest.h"
#include "MaterialCompiler.h"

UMaterialExpressionGerstnerWave::UMaterialExpressionGerstnerWave(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	MenuCategories.Add(FText::FromString(TEXT("Estranged")));
#endif

	Outputs.Reset();
	Outputs.Add(FExpressionOutput(TEXT("Offsets")));
	Outputs.Add(FExpressionOutput(TEXT("Normal")));
}

#if WITH_EDITOR

int32 UMaterialExpressionGerstnerWave::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	if (!WaterManifest)
	{
		Compiler->Error(TEXT("Missing Water Manifest"));
		return INDEX_NONE;
	}

	// 1. Compile Inputs (Common for both outputs)
	int32 WorldPosIndex = WorldPosition.GetTracedInput().Expression 
		? WorldPosition.Compile(Compiler) 
		: Compiler->WorldPosition(EWorldPositionIncludedOffsets::WPT_Default);

	int32 TimeIndex = Time.GetTracedInput().Expression 
		? Time.Compile(Compiler) 
		: Compiler->GameTime(false, 0.0f); 

	int32 IntensityIndex = GlobalIntensity.GetTracedInput().Expression
		? GlobalIntensity.Compile(Compiler)
		: Compiler->Constant(1.0f);

	// 2. Build Math Graph
	
	// Prepare Accumulators
	// For Output 0 (Offsets): X, Y, Z accumulators
	// For Output 1 (Normal): X, Y, Z accumulators
	int32 AccX = Compiler->Constant(0.0f);
	int32 AccY = Compiler->Constant(0.0f);
	int32 AccZ = (OutputIndex == 0) ? Compiler->Constant(0.0f) : Compiler->Constant(1.0f); // Normal starts at (0,0,1)

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
		
		int32 NodeAmp = Compiler->Mul(Compiler->Constant(Wave.Amplitude), IntensityIndex);
		int32 NodeSteep = Compiler->Constant(Wave.Steepness);

		// Phase Calculation
		int32 DotP = Compiler->Add(
			Compiler->Mul(NodeDirX, PosX),
			Compiler->Mul(NodeDirY, PosY)
		);

		int32 NodeCK = Compiler->Mul(NodeC, NodeK);
		int32 Phase = Compiler->Sub(
			Compiler->Mul(NodeK, DotP),
			Compiler->Mul(NodeCK, TimeIndex)
		);

		int32 SinP = Compiler->Sine(Phase);
		int32 CosP = Compiler->Cosine(Phase);

		if (OutputIndex == 0)
		{
			// --- OFFSET CALCULATION ---
			// Z += Amplitude * CosP
			AccZ = Compiler->Add(AccZ, Compiler->Mul(NodeAmp, CosP));

			// XY += Steepness * Amplitude * Dir * SinP
			int32 WA = Compiler->Mul(NodeSteep, NodeAmp);
			int32 CommonTerm = Compiler->Mul(WA, SinP);

			AccX = Compiler->Add(AccX, Compiler->Mul(CommonTerm, NodeDirX));
			AccY = Compiler->Add(AccY, Compiler->Mul(CommonTerm, NodeDirY));
		}
		else if (OutputIndex == 1)
		{
			// --- NORMAL CALCULATION ---
			// N.xy -= Dir * K * A * SinP
			// N.z -= Steepness * K * Amp * CosP
			
			// Term = K * Amp
			int32 Term = Compiler->Mul(NodeK, NodeAmp);
			
			// XY Term = Term * SinP * Dir
			int32 XYCommon = Compiler->Mul(Term, SinP);
			
			// We Use ADD for XY based on previous reasoning that N.x ~ Sin for x-displacement ~ Sin.
			AccX = Compiler->Add(AccX, Compiler->Mul(XYCommon, NodeDirX));
			AccY = Compiler->Add(AccY, Compiler->Mul(XYCommon, NodeDirY));

			// Z Term
			int32 ZTerm = Compiler->Mul(Compiler->Mul(NodeSteep, NodeAmp), NodeK);
			AccZ = Compiler->Sub(AccZ, Compiler->Mul(ZTerm, CosP));
		}
	}

	if (OutputIndex == 1)
	{
		// Normalize the result for safety
		int32 ResultVec = Compiler->AppendVector(
			Compiler->AppendVector(AccX, AccY),
			AccZ
		);
		return Compiler->Normalize(ResultVec);
	}
	
	// OutputIndex 0 -> Offsets
	int32 Result = Compiler->AppendVector(
		Compiler->AppendVector(AccX, AccY),
		AccZ
	);

	return Result;
}

void UMaterialExpressionGerstnerWave::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("Estranged Gerstner Waves"));
}

#endif