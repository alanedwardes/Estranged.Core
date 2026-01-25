#include "Materials/MaterialExpressionVertexFlow.h"
#include "MaterialCompiler.h"

UMaterialExpressionVertexFlow::UMaterialExpressionVertexFlow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	MenuCategories.Add(FText::FromString(TEXT("Estranged")));
#endif

	Outputs.Reset();
	Outputs.Add(FExpressionOutput(TEXT("UV A")));
	Outputs.Add(FExpressionOutput(TEXT("UV B")));
	Outputs.Add(FExpressionOutput(TEXT("Alpha")));

	bShowOutputNameOnPin = true;
}

#if WITH_EDITOR

int32 UMaterialExpressionVertexFlow::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	int32 BaseUVIndex = BaseUV.GetTracedInput().Expression 
		? BaseUV.Compile(Compiler) 
		: Compiler->TextureCoordinate(0, false, false);

	int32 SpeedIndex = FlowSpeed.GetTracedInput().Expression 
		? FlowSpeed.Compile(Compiler) 
		: Compiler->Constant(1.0f);

	int32 StrengthIndex = FlowStrength.GetTracedInput().Expression 
		? FlowStrength.Compile(Compiler) 
		: Compiler->Constant(0.2f);

	int32 TimeIndex = Time.GetTracedInput().Expression 
		? Time.Compile(Compiler) 
		: Compiler->GameTime(false, 0.0f);

	// Phase Calculation
	// Phase0 = Frac(Time * Speed + PhaseOffset)
	int32 PhaseOffsetIndex = PhaseOffset.GetTracedInput().Expression 
		? PhaseOffset.Compile(Compiler) 
		: Compiler->Constant(0.0f);

	int32 ScaledTime = Compiler->Mul(TimeIndex, SpeedIndex);
	ScaledTime = Compiler->Add(ScaledTime, PhaseOffsetIndex);

	int32 Phase0 = Compiler->Frac(ScaledTime);
	
	// Phase1 = Frac(Time * Speed + PhaseOffset + 0.5)
	int32 Phase1 = Compiler->Frac(Compiler->Add(ScaledTime, Compiler->Constant(0.5f)));

	// Vertex Color Flow Decode
	// Flow = (VertexColor.RG * 2 - 1) * Strength
	int32 VertColor = Compiler->VertexColor();
	int32 VertRG = Compiler->ComponentMask(VertColor, true, true, false, false);
	int32 RemappedFlow = Compiler->Sub(
		Compiler->Mul(VertRG, Compiler->Constant(2.0f)),
		Compiler->Constant(1.0f)
	);
	int32 FinalFlowVector = Compiler->Mul(RemappedFlow, StrengthIndex);

	// Output Branching
	if (OutputIndex == 0) // UV A
	{
		// UV A = BaseUV + Flow * Phase0
		int32 FlowOffset0 = Compiler->Mul(FinalFlowVector, Phase0);
		return Compiler->Add(BaseUVIndex, FlowOffset0);
	}
	else if (OutputIndex == 1) // UV B
	{
		// UV B = BaseUV + Flow * Phase1
		int32 FlowOffset1 = Compiler->Mul(FinalFlowVector, Phase1);
		return Compiler->Add(BaseUVIndex, FlowOffset1);
	}
	else if (OutputIndex == 2) // Alpha
	{
		// Alpha = Abs(Phase0 * 2 - 1)
		// 0 = Phase0 is at 0.5 (Midpoint/Safe) -> Show UV A
		// 1 = Phase0 is at 0/1 (Resetting/Pop)  -> Show UV B
		return Compiler->Abs(
			Compiler->Sub(
				Compiler->Mul(Phase0, Compiler->Constant(2.0f)),
				Compiler->Constant(1.0f)
			)
		);
	}

	return Compiler->Constant(0.0f);
}

void UMaterialExpressionVertexFlow::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("Estranged Vertex Flow"));
}

#endif
