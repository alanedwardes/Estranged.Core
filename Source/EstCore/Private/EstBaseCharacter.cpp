#include "EstBaseCharacter.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "EstCore.h"
#include "EstBaseWeapon.h"
#include "EstCharacterMovementComponent.h"
#include "EstCapabilityVolume.h"
#include "EstHealthComponent.h"
#include "EstFirearmWeapon.h"

AEstBaseCharacter::AEstBaseCharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UEstCharacterMovementComponent>(CharacterMovementComponentName))
{
	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UEstHealthComponent>(this, TEXT("Health"));

	EstCharacterMovement = Cast<UEstCharacterMovementComponent>(GetCharacterMovement());
}

void AEstBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (EquippedWeapon.IsValid() && !EquippedWeapon->IsEquipped())
	{
		EquipWeapon_Implementation(EquippedWeapon.Get());
	}
}

void AEstBaseCharacter::EquipWeapon_Implementation(AEstBaseWeapon* Weapon)
{
	UnequipWeapon();

	EquippedWeapon = Weapon;
	EquippedWeapon->Equip(this);
	OnChangeWeapon.Broadcast(EquippedWeapon.Get());

	if (GetMesh())
	{
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, Weapon->SocketName);
	}

	Weapon->OnPrimaryAttack.AddDynamic(this, &AEstBaseCharacter::PlayPrimaryFire);
	Weapon->OnSecondaryAttack.AddDynamic(this, &AEstBaseCharacter::PlaySecondaryFire);
	AEstFirearmWeapon* Firearm = Cast<AEstFirearmWeapon>(Weapon);
	if (Firearm != nullptr)
	{
		Firearm->OnReload.AddDynamic(this, &AEstBaseCharacter::PlayReload);
	}
}

void AEstBaseCharacter::PlayPrimaryFire()
{
	if (HasWeapon() && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(EquippedWeapon->CharacterAnimManifest.PrimaryAttack);
	}
}

void AEstBaseCharacter::PlaySecondaryFire()
{
	if (HasWeapon() && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(EquippedWeapon->CharacterAnimManifest.SecondaryAttack);
	}
}

void AEstBaseCharacter::PlayReload()
{
	if (HasWeapon() && GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(EquippedWeapon->CharacterAnimManifest.Reload);
	}
}

void AEstBaseCharacter::UnequipWeapon_Implementation()
{
	if (!HasWeapon())
	{
		return;
	}

	EquippedWeapon->OnPrimaryAttack.RemoveAll(this);
	EquippedWeapon->OnSecondaryAttack.RemoveAll(this);
	AEstFirearmWeapon* Firearm = Cast<AEstFirearmWeapon>(EquippedWeapon.Get());
	if (Firearm != nullptr)
	{
		Firearm->OnReload.RemoveAll(this);
	}

	OnChangeWeapon.Broadcast(nullptr);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	EquippedWeapon->Unequip();
	EquippedWeapon.Reset();
}

bool AEstBaseCharacter::HasWeapon()
{
	return EquippedWeapon.IsValid() && EquippedWeapon->IsEquipped();
}

float AEstBaseCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	float DamageReceived = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (HealthComponent->IsDepleted() && !ActorHasTag(TAG_DEAD))
	{
		UE_LOG(LogClass, Log, TEXT("Character %s just died"), *GetName());
		Tags.Add(TAG_DEAD);

		if (!IsPlayerControlled())
		{
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (HasWeapon())
		{
			UnequipWeapon();
		}
	}

	return DamageReceived;
}

bool AEstBaseCharacter::CanJumpInternal_Implementation() const
{
	if (Controller->IsMoveInputIgnored())
	{
		return false;
	}

	const AEstCapabilityVolume* Capabilities = Cast<AEstCapabilityVolume>(GetPawnPhysicsVolume());
	if (Capabilities == nullptr)
	{
		return Super::CanJumpInternal_Implementation();
	}

	return Capabilities->CanJump && Super::CanJumpInternal_Implementation();
}

bool AEstBaseCharacter::CanCrouch()
{
	if (Controller->IsMoveInputIgnored())
	{
		return false;
	}

	const AEstCapabilityVolume* Capabilities = Cast<AEstCapabilityVolume>(GetPawnPhysicsVolume());
	if (Capabilities == nullptr)
	{
		return Super::CanCrouch();
	}

	return Capabilities->CanCrouch && Super::CanCrouch();
}

bool AEstBaseCharacter::CanSprint()
{
	const AEstCapabilityVolume* Capabilities = Cast<AEstCapabilityVolume>(GetPawnPhysicsVolume());
	if (Capabilities == nullptr)
	{
		return EstCharacterMovement->CanEverSprint();
	}

	return Capabilities->CanSprint && EstCharacterMovement->CanEverSprint();
}
