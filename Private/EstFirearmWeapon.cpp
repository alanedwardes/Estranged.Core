#include "EstCore.h"
#include "EstFirearmAmunition.h"
#include "EstFirearmWeapon.h"
#include "Runtime/AIModule/Classes/Perception/AISense_Hearing.h"

AEstFirearmWeapon::AEstFirearmWeapon(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryAmmunition = PCIP.CreateDefaultSubobject<UEstFirearmAmunition>(this, TEXT("PrimaryAmmunition"));
	SecondaryAmmunition = PCIP.CreateDefaultSubobject<UEstFirearmAmunition>(this, TEXT("SecondaryAmmunition"));
	MuzzleFlash = PCIP.CreateDefaultSubobject<USpotLightComponent>(this, TEXT("MuzzleFlash"));
	MuzzleFlash->SetLightColor(FLinearColor(1.f, .5f, 0.f));
	MuzzleFlash->SetIntensity(4.f);
	MuzzleFlash->SetAttenuationRadius(3096.f);
	MuzzleFlash->SetCastShadows(false);
	MuzzleFlash->SetVisibility(false);
	MuzzleFlash->SetInnerConeAngle(0.f);
	MuzzleFlash->SetOuterConeAngle(90.f);
	MuzzleFlash->SetLightFalloffExponent(2.0f);
	MuzzleFlash->bUseInverseSquaredFalloff = false;
	MuzzleFlash->AttachToComponent(WeaponMesh, FAttachmentTransformRules::KeepRelativeTransform);

	PrimaryFireTime = -1.f;
}

void AEstFirearmWeapon::PrimaryAttack()
{
	if (AEstBaseWeapon::IsEngagedInActivity())
	{
		return;
	}

	// If we're out but have mags, reload
	if (!PrimaryAmmunition->CanFire() && PrimaryAmmunition->CanReload())
	{
		return Reload();
	}

	// If we're doing the block for semi-automatic or no ammo
	if (bSemiBlocking || !PrimaryAmmunition->CanFire())
	{
		return;
	}

	PrimaryFireTime = GetWorld()->TimeSeconds;
	AEstBaseWeapon::PrimaryAttack();
	PrimaryAmmunition->Fire();

	PrimaryFireEffects();

	if (bIsSemiAutomatic)
	{
		bSemiBlocking = true;
	}
}

void AEstFirearmWeapon::PrimaryFireEffects()
{
	MuzzleFlash->SetVisibility(true);

	// Report the gunshot to the perception system
	UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 1.f, GetOwner(), 1024.f);
}

void AEstFirearmWeapon::SecondaryAttack()
{
	if (AEstBaseWeapon::IsEngagedInActivity())
	{
		return;
	}

	// If we're out but have mags, reload
	if (!SecondaryAmmunition->CanFire() && SecondaryAmmunition->CanReload())
	{
		return Reload();
	}

	// If no ammo
	if (!SecondaryAmmunition->CanFire())
	{
		return;
	}

	AEstBaseWeapon::SecondaryAttack();
	SecondaryAmmunition->Fire();
}

void AEstFirearmWeapon::Reload()
{
	if (IsEngagedInActivity())
	{
		return;
	}

	if (!PrimaryAmmunition->CanReload())
	{
		return;
	}

	// Fire the event first, then do the maths
	OnReload.Broadcast();

	if (WeaponAnimManifest.Reload != nullptr)
	{
		WeaponMesh->GetAnimInstance()->Montage_Play(WeaponAnimManifest.Reload);
		SetEngagedInActivity(ReloadLength);
	}
	PrimaryAmmunition->Reload();
}

bool AEstFirearmWeapon::IsReloading()
{
	if (WeaponAnimManifest.Reload == nullptr)
	{
		return false;
	}

	if (!IsEngagedInActivity())
	{
		return false;
	}

	return WeaponMesh->GetAnimInstance()->Montage_IsPlaying(WeaponAnimManifest.Reload);
}

void AEstFirearmWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->TimeSince(ReloadExpireTime) < 0)
	{
		Reload();
	}

	if (MuzzleFlash->IsVisible() && GetWorld()->TimeSince(PrimaryFireTime) > .05f)
	{
		MuzzleFlash->SetVisibility(false);
	}
}

void AEstFirearmWeapon::ReloadPressed()
{
	if (PrimaryAmmunition->CanReload())
	{
		// Issue a reload command, expiring in T + primary attack time
		ReloadExpireTime = GetWorld()->TimeSeconds + PrimaryAttackLength;
	}
}

void AEstFirearmWeapon::PrimaryAttackEnd()
{
	AEstBaseWeapon::PrimaryAttackEnd();
	bSemiBlocking = false;
}
