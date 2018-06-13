#pragma once

#include "GameFramework/Actor.h"
#include "EstFogParameters.generated.h"

USTRUCT(BlueprintType)
struct FEstFogParameters
{
	GENERATED_BODY()

	FEstFogParameters()
	{
		FogInscatteringColor = FLinearColor(0.447f, 0.638f, 1.0f);
		FogDensity = 0.02f;
		FogHeightFalloff = 0.2f;
		StartDistance = 0.0f;
	}

	UPROPERTY(EditAnywhere, SaveGame, BlueprintReadWrite, Category = "Fog")
	FLinearColor FogInscatteringColor;

	UPROPERTY(EditAnywhere, SaveGame, BlueprintReadWrite, Category = "Fog", meta = (UIMin = "0", UIMax = "5000", EditCondition = "!bEnableVolumetricFog"))
	float StartDistance;

	UPROPERTY(EditAnywhere, SaveGame, BlueprintReadWrite, Category = "Fog", meta = (UIMin = "0", UIMax = ".05"))
	float FogDensity;

	UPROPERTY(EditAnywhere, SaveGame, BlueprintReadWrite, Category = "Fog", meta = (UIMin = "0.001", UIMax = "2"))
	float FogHeightFalloff;

	friend FArchive& operator<<(FArchive& Archive, FEstFogParameters& FogParameters)
	{
		Archive << FogParameters.FogInscatteringColor;
		Archive << FogParameters.StartDistance;
		Archive << FogParameters.FogDensity;
		Archive << FogParameters.FogHeightFalloff;
		return Archive;
	}
};