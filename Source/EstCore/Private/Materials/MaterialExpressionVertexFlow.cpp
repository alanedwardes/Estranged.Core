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

	int32 PhaseOffsetIndex = PhaseOffset.GetTracedInput().Expression 
		? PhaseOffset.Compile(Compiler) 
		: Compiler->Constant(0.0f);

	// Phase Calculation - consistent timing everywhere
	int32 ScaledTime = Compiler->Mul(TimeIndex, SpeedIndex);
	ScaledTime = Compiler->Add(ScaledTime, PhaseOffsetIndex);
	int32 Phase0 = Compiler->Frac(ScaledTime);
	int32 Phase1 = Compiler->Frac(Compiler->Add(ScaledTime, Compiler->Constant(0.5f)));

	// Flow Direction Input (from texture)
	// R = World X direction (0.5 = neutral, 0 = -X, 1 = +X)
	// G = World Y direction (0.5 = neutral, 0 = -Y, 1 = +Y)
	// B = Flow strength (0 = no flow, 1 = full flow)
	int32 FlowInput = FlowDirection.Compile(Compiler);
	int32 FlowRG = Compiler->ComponentMask(FlowInput, true, true, false, false);
	int32 FlowB = Compiler->ComponentMask(FlowInput, false, false, true, false);
	
	// Decode to -1 to 1 range
	int32 WorldFlowXY = Compiler->Sub(
		Compiler->Mul(FlowRG, Compiler->Constant(2.0f)),
		Compiler->Constant(1.0f)
	);
	
	// Build world-space flow vector (XY plane, Z=0)
	int32 WorldFlowX = Compiler->ComponentMask(WorldFlowXY, true, false, false, false);
	int32 WorldFlowY = Compiler->ComponentMask(WorldFlowXY, false, true, false, false);
	int32 WorldFlow3D = Compiler->AppendVector(
		Compiler->AppendVector(WorldFlowX, WorldFlowY),
		Compiler->Constant(0.0f)
	);
	
	// Get tangent frame for world-to-tangent transformation
	int32 Tangent = Compiler->VertexTangent();
	int32 Normal = Compiler->VertexNormal();
	int32 Bitangent = Compiler->Cross(Normal, Tangent);
	
	// Transform world flow to tangent space (UV space)
	int32 FlowU = Compiler->Dot(WorldFlow3D, Tangent);
	int32 FlowV = Compiler->Dot(WorldFlow3D, Bitangent);
	int32 TangentSpaceFlow = Compiler->AppendVector(FlowU, FlowV);
	
	// Apply B channel and strength
	int32 FlowWithBlendStrength = Compiler->Mul(TangentSpaceFlow, FlowB);
	int32 FinalFlowVector = Compiler->Mul(FlowWithBlendStrength, StrengthIndex);

	// Output Branching
	if (OutputIndex == 0) // UV A
	{
		int32 FlowOffset0 = Compiler->Mul(FinalFlowVector, Phase0);
		return Compiler->Add(BaseUVIndex, FlowOffset0);
	}
	else if (OutputIndex == 1) // UV B
	{
		int32 FlowOffset1 = Compiler->Mul(FinalFlowVector, Phase1);
		return Compiler->Add(BaseUVIndex, FlowOffset1);
	}
	else if (OutputIndex == 2) // Alpha
	{
		// Linear triangle wave: Abs(Phase0 * 2 - 1)
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
