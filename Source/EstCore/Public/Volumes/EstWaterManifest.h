#pragma once

#include "Engine/DataAsset.h"
#include "EstWaterManifest.generated.h"

UCLASS(BlueprintType)
class ESTCORE_API UEstWaterManifest : public UDataAsset
{
	GENERATED_BODY()

public:
	UEstWaterManifest(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		DamageType = UDamageType::StaticClass();
	}

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class USoundMix* SoundMixOverride;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UReverbEffect* ReverbOverride;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class TSubclassOf<UCameraModifier> CameraModifierOverride;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UPhysicalMaterial* PhysicalMaterialOverride;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UMaterialParameterCollection* ParameterCollection;

	/** Whether volume currently causes damage. */
	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	uint32 bPainCausing : 1;

	/** Damage done per second to actors in this volume when bPainCausing=true */
	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	float DamagePerSec;

	/** Type of damage done */
	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageType> DamageType;

	/** If pain causing, time between damage applications. */
	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	float PainInterval;

	/** If pain causing, the depth at which pain starts (zero is any depth) */
	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	float PainStartDepth;

	/** The radius at which to start the pain (zero is any radius) */
	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	float PainStartRadius;
};