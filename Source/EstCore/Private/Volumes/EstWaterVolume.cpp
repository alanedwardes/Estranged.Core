// Estranged is a trade mark of Alan Edwardes.


#include "Volumes/EstWaterVolume.h"
#include "EstCore.h"
#include "Components/BrushComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/ReverbEffect.h"
#include "Gameplay/EstPlayer.h"
#include "Gameplay/EstCharacterMovementComponent.h"
#include "Camera/CameraModifier.h"
#include "Engine/DamageEvents.h"

#define REVERB_TAG_UNDERWATER "Underwater"

AEstWaterVolume::AEstWaterVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GetBrushComponent()->SetCollisionProfileName(PROFILE_TRIGGER);
	FluidFriction = 0.35f;
	bWaterVolume = true;
	bPhysicsOnContact = false;

	Manifest = ObjectInitializer.CreateDefaultSubobject<UEstWaterManifest>(this, "WaterManifest");
}

void AEstWaterVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	AEstPlayer* Player = Cast<AEstPlayer>(OtherActor);
	if (IsValid(Player))
	{
		OverlappingPlayer = Player;

		SetActorTickEnabled(true);
		SetPlayerPaddling(true);
	}

	// Start timer if none is active
	if (!GetWorldTimerManager().IsTimerActive(TimerHandle_PainTimer))
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PainTimer, this, &AEstWaterVolume::PainTimer, Manifest->PainInterval, true);
	}
}

void AEstWaterVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	AEstPlayer* Player = Cast<AEstPlayer>(OtherActor);
	if (IsValid(Player))
	{
		SetActorTickEnabled(false);
		SetPlayerImmersed(false);
		SetPlayerPaddling(false);

		OverlappingPlayer = nullptr;
	}
}

void AEstWaterVolume::Tick(float DeltaTime)
{
	if (!IsValid(OverlappingPlayer))
	{
		return;
	}

	if (UEstGameplayStatics::AreActorsEyesInWater(OverlappingPlayer))
	{
		SetPlayerImmersed(true);
	}
	else
	{
		SetPlayerImmersed(false);
	}
}

void AEstWaterVolume::SetPlayerImmersed(bool bEnabled)
{
	if (bEnabled)
	{
		if (!bPlayerImmersed)
		{
			UGameplayStatics::PushSoundMixModifier(this, Manifest->SoundMixOverride);
			UGameplayStatics::ActivateReverbEffect(this, Manifest->ReverbOverride, REVERB_TAG_UNDERWATER);
			if (IsValid(OverlappingPlayer) && IsValid(OverlappingPlayer->PlayerCameraManager) && IsValid(Manifest->CameraModifierOverride))
			{
				OverlappingPlayer->PlayerCameraManager->AddNewCameraModifier(Manifest->CameraModifierOverride);
			}
			bPlayerImmersed = true;
		}
	}
	else
	{
		if (bPlayerImmersed)
		{
			UGameplayStatics::PopSoundMixModifier(this, Manifest->SoundMixOverride);
			UGameplayStatics::DeactivateReverbEffect(this, REVERB_TAG_UNDERWATER);
			if (IsValid(OverlappingPlayer) && IsValid(OverlappingPlayer->PlayerCameraManager) && IsValid(Manifest->CameraModifierOverride))
			{
				UCameraModifier* CameraModifier = OverlappingPlayer->PlayerCameraManager->FindCameraModifierByClass(Manifest->CameraModifierOverride);
				OverlappingPlayer->PlayerCameraManager->RemoveCameraModifier(CameraModifier);
			}
			bPlayerImmersed = false;
		}
	}
}

void AEstWaterVolume::SetPlayerPaddling(bool bEnabled)
{
	if (bEnabled)
	{
		if (!bPlayerPaddling)
		{
			if (IsValid(OverlappingPlayer) && IsValid(OverlappingPlayer->EstCharacterMovement))
			{
				OverlappingPlayer->EstCharacterMovement->FootstepMaterialOverride = Manifest->PhysicalMaterialOverride;
			}
			bPlayerPaddling = true;
		}
	}
	else
	{
		if (bPlayerPaddling)
		{
			if (IsValid(OverlappingPlayer) && IsValid(OverlappingPlayer->EstCharacterMovement))
			{
				OverlappingPlayer->EstCharacterMovement->FootstepMaterialOverride = nullptr;
			}
			bPlayerPaddling = false;
		}
	}
}

void AEstWaterVolume::ActorEnteredVolume(AActor* Other)
{
	Super::ActorEnteredVolume(Other);
}

void AEstWaterVolume::ActorLeavingVolume(AActor* Other)
{
	Super::ActorLeavingVolume(Other);
}

void AEstWaterVolume::PainTimer()
{
	if (Manifest->bPainCausing)
	{
		TSet<AActor*> TouchingActors;
		GetOverlappingActors(TouchingActors);

		for (AActor* const A : TouchingActors)
		{
			if (IsValid(A) && A->CanBeDamaged() && A->GetPhysicsVolume() == this)
			{
				CausePainTo(A);
			}
		}

		// Stop timer if nothing is overlapping us
		if (TouchingActors.Num() == 0)
		{
			GetWorldTimerManager().ClearTimer(TimerHandle_PainTimer);
		}
	}
}

void AEstWaterVolume::CausePainTo(AActor* Other)
{
	if (Manifest->DamagePerSec > 0.f)
	{
		TSubclassOf<UDamageType> DmgTypeClass = Manifest->DamageType ? *Manifest->DamageType : UDamageType::StaticClass();
		Other->TakeDamage(Manifest->DamagePerSec * Manifest->PainInterval, FDamageEvent(DmgTypeClass), nullptr, this);
	}
}