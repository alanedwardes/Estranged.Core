#pragma once

#include "Engine/DataAsset.h"
#include "EstWaterManifest.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FEstGerstnerWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	FVector2D Direction = FVector2D(1.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float Wavelength = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float Amplitude = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float Steepness = 0.5f;

	void Recompute() {}
};

UCLASS(BlueprintType)
class ESTCORE_API UEstWaterManifest : public UDataAsset
{
	GENERATED_BODY()

public:
	UEstWaterManifest(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class USoundMix* SoundMixOverride;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UReverbEffect* ReverbOverride;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class TSubclassOf<UCameraModifier> PaddlingCameraModifier;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class TSubclassOf<UCameraModifier> ImmersionCameraModifier;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class USoundBase* ImmersionLoop;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class USoundBase* ImmersionStart;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class USoundBase* ImmersionEnd;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UPhysicalMaterial* PhysicalMaterialOverride;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UMaterialParameterCollection* ParameterCollection;

	UPROPERTY(Category = "Water | Waves", EditAnywhere, BlueprintReadWrite)
	TArray<FEstGerstnerWave> Waves;

	UFUNCTION(BlueprintCallable, Category = "Water | Waves")
	float EvaluateWaveHeight(const FVector& WorldPosition, float Time) const;

	UFUNCTION(BlueprintCallable, Category = "Water | Waves")
	void EvaluateWaveOffsets(const FVector& WorldPosition, float Time, FVector& OutOffsets, FVector& OutNormal) const;

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

	UFUNCTION()
	void UpdateEffects(AEstPlayer* Player, FVector WaterSurface);

	UFUNCTION()
	void ActivatePaddlingEffects(class AEstPlayer* Player, FVector WaterSurface);

	UFUNCTION()
	void DeactivatePaddlingEffects(class AEstPlayer* Player);

	UFUNCTION()
	void ActivateImmersionEffects(class AEstPlayer* Player, FVector WaterSurface);

	UFUNCTION()
	void DeactivateImmersionEffects(class AEstPlayer* Player);
};