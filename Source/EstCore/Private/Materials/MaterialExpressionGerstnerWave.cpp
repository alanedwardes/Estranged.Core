#include "Materials/MaterialExpressionGerstnerWave.h"
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

	int32 ExcluderIndex = WaveExcluder.GetTracedInput().Expression
		? WaveExcluder.Compile(Compiler)
		: Compiler->Constant4(0.0f, 0.0f, 0.0f, 0.0f);

	// 2. Build Math Graph
	
	// Prepare Accumulators
	int32 AccX = Compiler->Constant(0.0f);
	int32 AccY = Compiler->Constant(0.0f);
	int32 AccZ = (OutputIndex == 0) ? Compiler->Constant(0.0f) : Compiler->Constant(1.0f); // Normal starts at (0,0,1)

	int32 PosX = Compiler->ComponentMask(WorldPosIndex, true, false, false, false);
	int32 PosY = Compiler->ComponentMask(WorldPosIndex, false, true, false, false);

	// Excluder Logic
	int32 ExcluderX = Compiler->ComponentMask(ExcluderIndex, true, false, false, false);
	int32 ExcluderY = Compiler->ComponentMask(ExcluderIndex, false, true, false, false);
	int32 ExcluderRad = Compiler->ComponentMask(ExcluderIndex, false, false, true, false);
	int32 ExcluderFade = Compiler->ComponentMask(ExcluderIndex, false, false, false, true);

	// Dist = Sqrt((PosX - ExX)^2 + (PosY - ExY)^2)
	int32 DiffX = Compiler->Sub(PosX, ExcluderX);
	int32 DiffY = Compiler->Sub(PosY, ExcluderY);
	int32 DistSq = Compiler->Add(Compiler->Mul(DiffX, DiffX), Compiler->Mul(DiffY, DiffY));
	int32 Dist = Compiler->SquareRoot(DistSq);

	// Alpha = SmoothStep(Rad, Rad + Fade, Dist)
	int32 FadeEnd = Compiler->Add(ExcluderRad, ExcluderFade);
	int32 AttenAlpha = Compiler->SmoothStep(ExcluderRad, FadeEnd, Dist);

	// 3. Process Packed Waves
	TArray<FExpressionInput*> PackedWaves;
	PackedWaves.Add(&Wave1);
	PackedWaves.Add(&Wave2);
	PackedWaves.Add(&Wave3);
	PackedWaves.Add(&Wave4);
	PackedWaves.Add(&Wave5);
	PackedWaves.Add(&Wave6);
	PackedWaves.Add(&Wave7);
	PackedWaves.Add(&Wave8);

	for (int32 i = 0; i < PackedWaves.Num(); ++i)
	{
		if (!PackedWaves[i]->GetTracedInput().Expression) continue;

		int32 WavePack = PackedWaves[i]->Compile(Compiler);
		
		// Unpack: X=DirX*K, Y=DirY*K, Z=Amp, W=Steepness
		int32 VK = Compiler->ComponentMask(WavePack, true, true, false, false);
		
		// K = length(VK). Add a small epsilon to avoid division by zero later.
		int32 NodeK = Compiler->SquareRoot(Compiler->Add(Compiler->Dot(VK, VK), Compiler->Constant(0.00001f)));
		
		int32 NodeAmp = Compiler->Mul(Compiler->ComponentMask(WavePack, false, false, true, false), IntensityIndex);
		NodeAmp = Compiler->Mul(NodeAmp, AttenAlpha); // Apply Excluder Attenuation

		int32 NodeSteep = Compiler->ComponentMask(WavePack, false, false, false, true);
		
		// Phase = dot(VK, Pos) - sqrt(9.8 * K) * Time
		int32 DotP = Compiler->Dot(VK, Compiler->AppendVector(PosX, PosY));
		int32 Omega = Compiler->SquareRoot(Compiler->Mul(Compiler->Constant(980.0f), NodeK));
		int32 Phase = Compiler->Sub(DotP, Compiler->Mul(Omega, TimeIndex));

		int32 SinP = Compiler->Sine(Phase);
		int32 CosP = Compiler->Cosine(Phase);

		if (OutputIndex == 0)
		{
			// Normalized Direction = VK / K
			int32 NodeDir = Compiler->Div(VK, NodeK);
			int32 NodeDirX = Compiler->ComponentMask(NodeDir, true, false, false, false);
			int32 NodeDirY = Compiler->ComponentMask(NodeDir, false, true, false, false);

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
			// Normal XY Term = K * Amp * SinP * Dir = Amp * SinP * VK
			// Note: (VK / K) * K cancels out, avoiding a division.
			int32 XYCommon = Compiler->Mul(NodeAmp, SinP);
			
			AccX = Compiler->Add(AccX, Compiler->Mul(XYCommon, Compiler->ComponentMask(VK, true, false, false, false)));
			AccY = Compiler->Add(AccY, Compiler->Mul(XYCommon, Compiler->ComponentMask(VK, false, true, false, false)));

			// Z += Steepness * K * Amp * CosP
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