#include "Volumes/EstWaterManifest.h"
#include "Gameplay/EstFootstepComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Gameplay/EstPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraModifier.h"
#include "Components/AudioComponent.h"

#define REVERB_TAG_UNDERWATER "Underwater"
#define WATER_SURFACE_EYES_MATERIAL_PARAMETER "WaterSurfaceAtEyes"
#define WATER_SURFACE_MATERIAL_PARAMETER "WaterSurface"
#define SOUND_TAG_UNDERWATER "UnderwaterLoopSound"

void UEstWaterManifest::UpdateEffects(AEstPlayer* Player, FVector WaterSurface, FVector VolumeExtent, float WaveIntensity)
{
	UKismetMaterialLibrary::SetScalarParameterValue(Player, ParameterCollection, WATER_SURFACE_EYES_MATERIAL_PARAMETER, WaterSurface.Z);
	UKismetMaterialLibrary::SetScalarParameterValue(Player, ParameterCollection, WATER_SURFACE_MATERIAL_PARAMETER, VolumeExtent.Z);
}

UEstWaterManifest::UEstWaterManifest(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UEstWaterManifest::GenerateWaves()
{
	Waves.Empty();

	FRandomStream Stream(GeneratorProfile.RandomSeed);

	TArray<float> Wavelengths;
	TArray<float> Steepnesses;
	TArray<float> Weightings;
	float TotalWeight = 0.0f;

	for (int32 i = 0; i < GeneratorProfile.NumWaves; ++i)
	{
		float WL = Stream.FRandRange(GeneratorProfile.MinWavelength, GeneratorProfile.MaxWavelength);
		Wavelengths.Add(WL);
		
		float ST = Stream.FRandRange(GeneratorProfile.MinSteepness, GeneratorProfile.MaxSteepness);
		Steepnesses.Add(ST);

		// Weighting based on wavelength. Longer waves carry more amplitude.
		float Weight = FMath::Pow(WL, GeneratorProfile.WavelengthPower);
		Weightings.Add(Weight);
		TotalWeight += Weight;
	}

	for (int32 i = 0; i < GeneratorProfile.NumWaves; ++i)
	{
		FEstGerstnerWave NewWave;
		NewWave.Wavelength = Wavelengths[i];
		NewWave.Steepness = Steepnesses[i];

		// Proportional amplitude based on weighting
		if (TotalWeight > 0.0f)
		{
			NewWave.Amplitude = (Weightings[i] / TotalWeight) * GeneratorProfile.OverallAmplitude;
		}
		else
		{
			NewWave.Amplitude = GeneratorProfile.OverallAmplitude / GeneratorProfile.NumWaves;
		}

		float Variance = Stream.FRandRange(-GeneratorProfile.DirectionVariance, GeneratorProfile.DirectionVariance);
		NewWave.Angle = FRotator::ClampAxis(GeneratorProfile.WindDirection + Variance);

		Waves.Add(NewWave);
	}
}

float UEstWaterManifest::EvaluateWaveHeight(const FVector& WorldPosition, float Time, float WaveIntensity) const
{
	FVector Offsets, Normal;
	EvaluateWaveOffsets(WorldPosition, Time, Offsets, Normal, WaveIntensity);
	return WorldPosition.Z + Offsets.Z;
}

void UEstWaterManifest::EvaluateWaveOffsets(const FVector& WorldPosition, float Time, FVector& OutOffsets, FVector& OutNormal, float WaveIntensity) const
{
	OutOffsets = FVector::ZeroVector;
	OutNormal = FVector::UpVector; // Simplified normal accumulation

	const float Gravity = 980.0f;
	const FVector2D Pos2D(WorldPosition.X, WorldPosition.Y);

	float AccX = 0.0f;
	float AccY = 0.0f;
	float AccZ = 1.0f; // Normal starts at (0,0,1)

	for (const FEstGerstnerWave& Wave : Waves)
	{
		if (Wave.Wavelength <= KINDA_SMALL_NUMBER) continue;

		const float AngleRad = FMath::DegreesToRadians(Wave.Angle);
		const FVector2D Direction(FMath::Cos(AngleRad), FMath::Sin(AngleRad));

		const float K = 2.0f * UE_PI / Wave.Wavelength;
		const float C = FMath::Sqrt(Gravity / K); // Phase speed
		
		const float DotP = (Direction.X * Pos2D.X) + (Direction.Y * Pos2D.Y);
		const float Phase = K * DotP - (C * K * Time);
		
		float SinP, CosP;
		FMath::SinCos(&SinP, &CosP, Phase);

		const float Amplitude = Wave.Amplitude * WaveIntensity;

		// Z Displacement (Height)
		OutOffsets.Z += Amplitude * CosP;

		// XY Displacement
		const float WA = Wave.Steepness * Amplitude;
		OutOffsets.X += WA * Direction.X * SinP;
		OutOffsets.Y += WA * Direction.Y * SinP;

		// Normal Calculation: -K * Amp * Sin(Phase) * Direction
		// This matches the partial derivatives used on GPU
		const float XYCommon = Amplitude * SinP;
		AccX += XYCommon * (Direction.X * K);
		AccY += XYCommon * (Direction.Y * K);

		// Z component: 1 - sum(Steepness * K * Amp * Cos(Phase))
		const float ZTerm = WA * K;
		AccZ -= ZTerm * CosP;
	}

	OutNormal = FVector(AccX, AccY, AccZ).GetSafeNormal();
}

void UEstWaterManifest::ActivatePaddlingEffects(AEstPlayer* Player)
{
	if (!IsValid(Player))
	{
		return;
	}

	if (IsValid(Player->FootstepComponent))
	{
		Player->FootstepComponent->FootstepMaterialOverride = PhysicalMaterialOverride;
	}

	if (IsValid(Player->PlayerCameraManager) && IsValid(PaddlingCameraModifier))
	{
		Player->PlayerCameraManager->AddNewCameraModifier(PaddlingCameraModifier);
	}
}

void UEstWaterManifest::DeactivatePaddlingEffects(AEstPlayer* Player)
{
	if (!IsValid(Player))
	{
		return;
	}

	if (IsValid(Player->FootstepComponent))
	{
		Player->FootstepComponent->FootstepMaterialOverride = nullptr;
	}

	if (IsValid(Player->PlayerCameraManager) && IsValid(PaddlingCameraModifier))
	{
		UCameraModifier* CameraModifier = Player->PlayerCameraManager->FindCameraModifierByClass(PaddlingCameraModifier);
		Player->PlayerCameraManager->RemoveCameraModifier(CameraModifier);
	}
}

void UEstWaterManifest::ActivateImmersionEffects(AEstPlayer* Player)
{
	if (!IsValid(Player))
	{
		return;
	}

	UGameplayStatics::PushSoundMixModifier(Player, SoundMixOverride);
	UGameplayStatics::ActivateReverbEffect(Player, ReverbOverride, REVERB_TAG_UNDERWATER);
	UGameplayStatics::PlaySound2D(Player, ImmersionStart);

	if (IsValid(ImmersionLoop))
	{
		UAudioComponent* UnderwaterLoop = UGameplayStatics::SpawnSoundAttached(ImmersionLoop, Player->GetRootComponent(), NAME_None);
		UnderwaterLoop->ComponentTags.Add(SOUND_TAG_UNDERWATER);
	}

	if (IsValid(Player->PlayerCameraManager) && IsValid(ImmersionCameraModifier))
	{
		Player->PlayerCameraManager->AddNewCameraModifier(ImmersionCameraModifier);
	}
}

void UEstWaterManifest::DeactivateImmersionEffects(AEstPlayer* Player)
{
	if (!IsValid(Player))
	{
		return;
	}

	UGameplayStatics::PopSoundMixModifier(Player, SoundMixOverride);
	UGameplayStatics::DeactivateReverbEffect(Player, REVERB_TAG_UNDERWATER);
	UGameplayStatics::PlaySound2D(Player, ImmersionEnd);

	Player->ForEachComponent<UAudioComponent>(false, [&](UAudioComponent* AudioComponent)
	{
		if (IsValid(AudioComponent) && AudioComponent->ComponentHasTag(SOUND_TAG_UNDERWATER))
		{
			// Stop will invoke destroy
			AudioComponent->Stop();
		}
	});

	if (IsValid(Player->PlayerCameraManager) && IsValid(ImmersionCameraModifier))
	{
		UCameraModifier* CameraModifier = Player->PlayerCameraManager->FindCameraModifierByClass(ImmersionCameraModifier);
		Player->PlayerCameraManager->RemoveCameraModifier(CameraModifier);
	}
}
