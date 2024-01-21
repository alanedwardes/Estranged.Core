#include "Volumes/EstWaterManifest.h"
#include "Gameplay/EstCharacterMovementComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Gameplay/EstPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraModifier.h"
#include "Components/AudioComponent.h"

#define REVERB_TAG_UNDERWATER "Underwater"
#define WATER_SURFACE_MATERIAL_PARAMETER "WaterSurface"
#define SOUND_TAG_UNDERWATER "UnderwaterLoopSound"

void UEstWaterManifest::ActivatePaddlingEffects(AEstPlayer* Player, FVector WaterSurface)
{
	if (!IsValid(Player))
	{
		return;
	}

	if (IsValid(Player->EstCharacterMovement))
	{
		Player->EstCharacterMovement->FootstepMaterialOverride = PhysicalMaterialOverride;
	}

	UKismetMaterialLibrary::SetScalarParameterValue(Player, ParameterCollection, WATER_SURFACE_MATERIAL_PARAMETER, WaterSurface.Z);
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

	if (IsValid(Player->EstCharacterMovement))
	{
		Player->EstCharacterMovement->FootstepMaterialOverride = nullptr;
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
