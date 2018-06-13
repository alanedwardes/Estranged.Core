#include "EstCore.h"
#include "EstBaseCharacter.h"
#include "EstBaseWeapon.h"

AEstBaseWeapon::AEstBaseWeapon(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName("Item");
	WeaponMesh->bCastInsetShadow = true;
	SetRootComponent(WeaponMesh);
}

void AEstBaseWeapon::OnPostRestore_Implementation()
{
	if (OwnerActorName == NAME_None)
	{
		return;
	}

	for (TActorIterator<AEstBaseCharacter> Character(GetWorld()); Character; ++Character)
	{
		if (Character->GetFName() == OwnerActorName)
		{
			Character->EquipWeapon(this);
			return;
		}
	}
}

void AEstBaseWeapon::OnPreSave_Implementation()
{
	OwnerActorName = GetOwner() ? OwnerActorName = GetOwner()->GetFName() : NAME_None;
}

void AEstBaseWeapon::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	UpdateWeaponPhysicsState();
}

void AEstBaseWeapon::UpdateWeaponPhysicsState()
{
	if (IsEquipped())
	{
		WeaponMesh->SetSimulatePhysics(false);
		SetActorEnableCollision(false);
	}
	else
	{
		WeaponMesh->SetSimulatePhysics(true);
		SetActorEnableCollision(true);
	}
}

bool AEstBaseWeapon::OnUsed_Implementation(AEstBaseCharacter* User, class USceneComponent* UsedComponent)
{
	User->EquipWeapon(this);
	return true;
}

void AEstBaseWeapon::SetEngagedInActivity(float ActivityLength)
{
	ensure(!IsEngagedInActivity());
	LastActivityFinishTime = GetWorld()->TimeSeconds + ActivityLength;
}

void AEstBaseWeapon::PrimaryAttackStart()
{
	Unholster();
	bIsPrimaryFirePressed = true;
}

void AEstBaseWeapon::PrimaryAttackEnd()
{
	bIsPrimaryFirePressed = false;
}

void AEstBaseWeapon::SecondaryAttackStart()
{
	Unholster();
	bIsSecondaryFirePressed = true;
}

void AEstBaseWeapon::SecondaryAttackEnd()
{
	bIsSecondaryFirePressed = false;
}

void AEstBaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsPrimaryFirePressed)
	{
		return PrimaryAttack();
	}

	if (bIsSecondaryFirePressed)
	{
		return SecondaryAttack();
	}
}

void AEstBaseWeapon::PrimaryAttack()
{
	if (IsEngagedInActivity())
	{
		return;
	}

	if (WeaponAnimManifest.PrimaryAttack && WeaponMesh->GetAnimInstance())
	{
		WeaponMesh->GetAnimInstance()->Montage_Play(WeaponAnimManifest.PrimaryAttack);
		SetEngagedInActivity(PrimaryAttackLength);
	}

	OnPrimaryAttack.Broadcast();
}

void AEstBaseWeapon::SecondaryAttack()
{
	if (IsEngagedInActivity())
	{
		return;
	}

	if (WeaponAnimManifest.SecondaryAttack && WeaponMesh->GetAnimInstance())
	{
		WeaponMesh->GetAnimInstance()->Montage_Play(WeaponAnimManifest.SecondaryAttack);
		SetEngagedInActivity(SecondaryAttackLength);
	}

	OnSecondaryAttack.Broadcast();
}

void AEstBaseWeapon::Equip(AEstBaseCharacter* OwnerCharacter)
{
	SetOwner(OwnerCharacter);

	UpdateWeaponPhysicsState();
	OnEquip();
	Unholster();

	OnEquipped.Broadcast();
}

void AEstBaseWeapon::Unequip()
{
	const AActor* PreviousOwner = GetOwner();
	SetOwner(nullptr);

	bIsPrimaryFirePressed = false;
	bIsSecondaryFirePressed = false;

	if (WeaponMesh->GetAnimInstance())
	{
		WeaponMesh->GetAnimInstance()->Montage_Stop(0.f);
	}

	UpdateWeaponPhysicsState();

	FVector EyeLoc;
	FRotator EyeRot;
	PreviousOwner->GetActorEyesViewPoint(EyeLoc, EyeRot);
	SetActorLocationAndRotation(EyeLoc, EyeRot);

	if (!PreviousOwner->ActorHasTag(TAG_DEAD))
	{
		WeaponMesh->AddImpulse(EyeRot.Vector() * 1000.f);
	}

	OnUnequip();
}

bool AEstBaseWeapon::IsEquipped()
{
	if (!GetOwner())
	{
		return false;
	}

	AEstBaseCharacter* Character = Cast<AEstBaseCharacter>(GetOwner());
	if (!Character)
	{
		return false;
	}

	return Character->EquippedWeapon == this;
}

void AEstBaseWeapon::Holster()
{
	OnHolster();

	bIsHolstered = true;
}

void AEstBaseWeapon::Unholster()
{
	OnUnholster();

	bIsHolstered = false;
}