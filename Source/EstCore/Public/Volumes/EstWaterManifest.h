#pragma once

#include "Engine/DataAsset.h"
#include "EstWaterManifest.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FEstGerstnerWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (UIMin = 0, UIMax = 360, ToolTip = "Wave direction in degrees."))
	float Angle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (Units = "Centimeters", ToolTip = "Wave length"))
	float Wavelength = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (Units = "Centimeters", ToolTip = "Wave amplitude"))
	float Amplitude = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ToolTip = "Steepness ratio (0.0 to 1.0). Controls how sharp the wave peaks are.", UIMin = 0, UIMax = 1))
	float Steepness = 0.5f;

	void Recompute() {}
};

USTRUCT(BlueprintType)
struct ESTCORE_API FEstGerstnerWaveGeneratorProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator")
	int32 NumWaves = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator", meta = (Units = "Centimeters"))
	float MinWavelength = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator", meta = (Units = "Centimeters"))
	float MaxWavelength = 15000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator", meta = (Units = "Centimeters", ToolTip = "Total sum of all wave amplitudes. The peak-to-trough height will be roughly double this."))
	float OverallAmplitude = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator", meta = (ToolTip = "How much wavelength affects amplitude. 0 = equal amplitude, 1 = proportional to wavelength."))
	float WavelengthPower = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator", meta = (UIMin = 0, UIMax = 1))
	float MinSteepness = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator", meta = (UIMin = 0, UIMax = 1))
	float MaxSteepness = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator", meta = (UIMin = 0, UIMax = 360, Units = "Degrees"))
	float WindDirection = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator", meta = (UIMin = 0, UIMax = 180, Units = "Degrees"))
	float DirectionVariance = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Generator")
	int32 RandomSeed = 0;
};

UCLASS(BlueprintType)
class ESTCORE_API UEstWaterManifest : public UDataAsset
{
	GENERATED_BODY()

public:
	UEstWaterManifest(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Water | Generator")
	FEstGerstnerWaveGeneratorProfile GeneratorProfile;

	UFUNCTION(CallInEditor, Category = "Water | Generator")
	void GenerateWaves();

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

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UMaterialInterface* AboveWaterMaterial;

	UPROPERTY(Category = "Water", EditAnywhere, BlueprintReadWrite)
	class UMaterialInterface* BelowWaterMaterial;

	UPROPERTY(Category = "Water | Waves", EditAnywhere, BlueprintReadWrite)
	FEstGerstnerWave Wave1;

	UPROPERTY(Category = "Water | Waves", EditAnywhere, BlueprintReadWrite)
	FEstGerstnerWave Wave2;

	UPROPERTY(Category = "Water | Waves", EditAnywhere, BlueprintReadWrite)
	FEstGerstnerWave Wave3;

	UPROPERTY(Category = "Water | Waves", EditAnywhere, BlueprintReadWrite)
	FEstGerstnerWave Wave4;

	UPROPERTY(Category = "Water | Waves", EditAnywhere, BlueprintReadWrite)
	FEstGerstnerWave Wave5;

	UPROPERTY(Category = "Water | Waves", EditAnywhere, BlueprintReadWrite)
	FEstGerstnerWave Wave6;

	UPROPERTY(Category = "Water | Waves", EditAnywhere, BlueprintReadWrite)
	FEstGerstnerWave Wave7;

	UPROPERTY(Category = "Water | Waves", EditAnywhere, BlueprintReadWrite)
	FEstGerstnerWave Wave8;

	UFUNCTION(BlueprintCallable, Category = "Water | Waves")
	float EvaluateWaveHeight(const FVector& WorldPosition, float Time, float WaveIntensity = 1.0f) const;

	UFUNCTION(BlueprintCallable, Category = "Water | Waves")
	void EvaluateWaveOffsets(const FVector& WorldPosition, float Time, FVector& OutOffsets, FVector& OutNormal, float WaveIntensity = 1.0f) const;

	UFUNCTION()
	void UpdateEffects(AEstPlayer* Player, FVector WaterSurface, FVector VolumeExtent, float WaveIntensity);

	UFUNCTION()
	void ActivatePaddlingEffects(class AEstPlayer* Player);

	UFUNCTION()
	void DeactivatePaddlingEffects(class AEstPlayer* Player);

	UFUNCTION()
	void ActivateImmersionEffects(class AEstPlayer* Player);

	UFUNCTION()
	void DeactivateImmersionEffects(class AEstPlayer* Player);
};