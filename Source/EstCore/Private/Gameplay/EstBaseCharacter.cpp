#include "Gameplay/EstBaseCharacter.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "EstCore.h"
#include "Gameplay/EstBaseWeapon.h"
#include "Gameplay/EstCharacterMovementComponent.h"
#include "Volumes/EstCapabilityVolume.h"
#include "Gameplay/EstHealthComponent.h"
#include "Gameplay/EstGameInstance.h"
#include "Gameplay/EstGameplayStatics.h"

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
		OnDeath();
	}

	return DamageReceived;
}

void AEstBaseCharacter::OnPostRestore_Implementation()
{
	if (HealthComponent->IsDepleted())
	{
		OnDeath();
	}
}

void AEstBaseCharacter::OnDeath_Implementation()
{
	if (UEstGameplayStatics::GetEstGameInstance(this)->GetLoggerEnabled())
	{
		UEstGameplayStatics::GetEstGameInstance(this)->LogMessage(FEstLoggerMessage(this, EEstLoggerLevel::Normal, TEXT("Character just died")));
	}

	Tags.Add(TAG_DEAD);

	if (HasWeapon())
	{
		UnequipWeapon();
	}

	SetCanBeDamaged(false);
}

bool AEstBaseCharacter::CanJumpInternal_Implementation() const
{
	if (Controller->IsMoveInputIgnored())
	{
		return false;
	}

	const AEstCapabilityVolume* Capabilities = Cast<AEstCapabilityVolume>(GetPhysicsVolume());
	if (Capabilities == nullptr)
	{
		return Super::CanJumpInternal_Implementation();
	}

	return Capabilities->CanJump && Super::CanJumpInternal_Implementation();
}

bool AEstBaseCharacter::CanCrouch() const
{
	if (Controller->IsMoveInputIgnored())
	{
		return false;
	}

	const AEstCapabilityVolume* Capabilities = Cast<AEstCapabilityVolume>(GetPhysicsVolume());
	if (Capabilities == nullptr)
	{
		return Super::CanCrouch();
	}

	return Capabilities->CanCrouch && Super::CanCrouch();
}

bool AEstBaseCharacter::CanSprint()
{
	const AEstCapabilityVolume* Capabilities = Cast<AEstCapabilityVolume>(GetPhysicsVolume());
	if (Capabilities == nullptr)
	{
		return EstCharacterMovement->CanEverSprint();
	}

	return Capabilities->CanSprint && EstCharacterMovement->CanEverSprint();
}
