#include "Volumes/EstWaterManifest.h"
#include "Gameplay/EstFootstepComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Gameplay/EstPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraModifier.h"
#include "Components/AudioComponent.h"

#define REVERB_TAG_UNDERWATER "Underwater"
#define WATER_SURFACE_MATERIAL_PARAMETER "WaterSurface"
#define SOUND_TAG_UNDERWATER "UnderwaterLoopSound"

void UEstWaterManifest::UpdateEffects(AEstPlayer* Player, FVector WaterSurface)
{
	UKismetMaterialLibrary::SetScalarParameterValue(Player, ParameterCollection, WATER_SURFACE_MATERIAL_PARAMETER, WaterSurface.Z);
}



UEstWaterManifest::UEstWaterManifest(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DamageType = UDamageType::StaticClass();
}

float UEstWaterManifest::EvaluateWaveHeight(const FVector& WorldPosition, float Time) const
{
	FVector Offsets, Normal;
	EvaluateWaveOffsets(WorldPosition, Time, Offsets, Normal);
	return WorldPosition.Z + Offsets.Z;
}

void UEstWaterManifest::EvaluateWaveOffsets(const FVector& WorldPosition, float Time, FVector& OutOffsets, FVector& OutNormal) const
{
	OutOffsets = FVector::ZeroVector;
	OutNormal = FVector::UpVector; // Simplified normal accumulation

	const float Gravity = 980.0f;
	const FVector2D Pos2D(WorldPosition.X, WorldPosition.Y);

	for (const FEstGerstnerWave& Wave : Waves)
	{
		if (Wave.Wavelength <= KINDA_SMALL_NUMBER) continue;

		const float K = 2.0f * UE_PI / Wave.Wavelength;
		const float C = FMath::Sqrt(Gravity / K); // Phase speed
		
		const float DotP = (Wave.Direction.X * Pos2D.X) + (Wave.Direction.Y * Pos2D.Y);
		const float Phase = K * DotP - (C * K * Time);
		
		float SinP, CosP;
		FMath::SinCos(&SinP, &CosP, Phase);

		// Z Displacement (Height)
		OutOffsets.Z += Wave.Amplitude * CosP;

		// XY Displacement
		// QA = Steepness / K. Using simplified Q calculation:
		// If Steepness is 0-1 range, we generally want Q = Steepness / (Amplitude * K * NumWaves) to avoid loops.
		// Here assuming 'Steepness' is the raw Q factor for simplicity, or we can normalize.
		// Let's match typical direct control: X += Steepness * Amplitude * D * Sin
		
		const float WA = Wave.Steepness * Wave.Amplitude;
		OutOffsets.X += WA * Wave.Direction.X * SinP;
		OutOffsets.Y += WA * Wave.Direction.Y * SinP;
	}
}

void UEstWaterManifest::ActivatePaddlingEffects(AEstPlayer* Player, FVector WaterSurface)
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

void UEstWaterManifest::ActivateImmersionEffects(AEstPlayer* Player, FVector WaterSurface)
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
