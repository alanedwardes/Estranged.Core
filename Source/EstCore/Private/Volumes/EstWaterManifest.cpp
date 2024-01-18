#include "Volumes/EstWaterManifest.h"
#include "Gameplay/EstCharacterMovementComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Gameplay/EstPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraModifier.h"

#define REVERB_TAG_UNDERWATER "Underwater"
#define WATER_SURFACE_MATERIAL_PARAMETER "WaterSurface"

void UEstWaterManifest::ActivatePaddlingEffects(AEstPlayer* Player)
{
	if (IsValid(Player) && IsValid(Player->EstCharacterMovement))
	{
		Player->EstCharacterMovement->FootstepMaterialOverride = PhysicalMaterialOverride;
	}
}

void UEstWaterManifest::DeactivatePaddlingEffects(AEstPlayer* Player)
{
	if (IsValid(Player) && IsValid(Player->EstCharacterMovement))
	{
		Player->EstCharacterMovement->FootstepMaterialOverride = nullptr;
	}
}

void UEstWaterManifest::ActivateImmersionEffects(AEstPlayer* Player, float WaterSurfaceZ)
{
	UKismetMaterialLibrary::SetScalarParameterValue(this, ParameterCollection, WATER_SURFACE_MATERIAL_PARAMETER, WaterSurfaceZ);
	UGameplayStatics::PushSoundMixModifier(this, SoundMixOverride);
	UGameplayStatics::ActivateReverbEffect(this, ReverbOverride, REVERB_TAG_UNDERWATER);
	if (IsValid(Player) && IsValid(Player->PlayerCameraManager) && IsValid(CameraModifierOverride))
	{
		Player->PlayerCameraManager->AddNewCameraModifier(CameraModifierOverride);
	}
}

void UEstWaterManifest::DeactivateImmersionEffects(AEstPlayer* Player)
{
	UGameplayStatics::PopSoundMixModifier(this, SoundMixOverride);
	UGameplayStatics::DeactivateReverbEffect(this, REVERB_TAG_UNDERWATER);
	if (IsValid(Player) && IsValid(Player->PlayerCameraManager) && IsValid(CameraModifierOverride))
	{
		UCameraModifier* CameraModifier = Player->PlayerCameraManager->FindCameraModifierByClass(CameraModifierOverride);
		Player->PlayerCameraManager->RemoveCameraModifier(CameraModifier);
	}
}
