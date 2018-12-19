#include "EstCore.h"
#include "EstBaseCharacter.h"
#include "EstBaseWeapon.h"

AEstBaseWeapon::AEstBaseWeapon(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	bCanAim = true;

	WeaponMesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName("Item");
	WeaponMesh->bCastInsetShadow = true;
	SetRootComponent(WeaponMesh);

	if (GEngine != nullptr)
	{
		WeaponMesh->BodyInstance.bUseCCD = true;
	}
}

void AEstBaseWeapon::OnPostRestore_Implementation()
{
	if (!OwnerSaveId.IsValid())
	{
		return;
	}

	AActor* Owner = UEstGameplayStatics::FindActorBySaveIdInWorld(GetWorld(), OwnerSaveId);
	if (Owner == nullptr)
	{
		return;
	}

	AEstBaseCharacter* Character = Cast<AEstBaseCharacter>(Owner);
	if (Character == nullptr)
	{
		return;
	}

	Character->EquipWeapon(this);
}

void AEstBaseWeapon::OnPreSave_Implementation()
{
	OwnerSaveId = GetOwner() ? IEstSaveRestore::Execute_GetSaveId(GetOwner()) : FGuid();
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
	}

	if (SoundManifest.PrimaryAttack)
	{
		UGameplayStatics::SpawnSoundAttached(SoundManifest.PrimaryAttack, WeaponMesh);
	}

	SetEngagedInActivity(OwnerCharacter->IsPlayerControlled() ? PrimaryAttackLengthPlayer : PrimaryAttackLengthAI);

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
	}

	if (SoundManifest.SecondaryAttack)
	{
		UGameplayStatics::SpawnSoundAttached(SoundManifest.SecondaryAttack, WeaponMesh);
	}

	SetEngagedInActivity(OwnerCharacter->IsPlayerControlled() ? SecondaryAttackLengthPlayer : SecondaryAttackLengthAI);

	OnSecondaryAttack.Broadcast();
}

void AEstBaseWeapon::Equip(AEstBaseCharacter* NewOwnerCharacter)
{
	SetOwner(NewOwnerCharacter);

	OwnerCharacter = NewOwnerCharacter;

	if (EquippedWeaponAnimClass)
	{
		WeaponMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		WeaponMesh->SetAnimInstanceClass(EquippedWeaponAnimClass);
	}

	if (EquipSound)
	{
		UGameplayStatics::SpawnSoundAttached(EquipSound, WeaponMesh);
	}

	UEstGameplayStatics::SetHighlightState(this, false);

	UpdateWeaponPhysicsState();
	OnEquip();
	Unholster();

	OnEquipped.Broadcast();
}

void AEstBaseWeapon::Unequip()
{
	const AActor* PreviousOwner = GetOwner();
	SetOwner(nullptr);

	OwnerCharacter = nullptr;

	bIsPrimaryFirePressed = false;
	bIsSecondaryFirePressed = false;

	if (WeaponMesh->GetAnimInstance())
	{
		WeaponMesh->GetAnimInstance()->Montage_Stop(0.f);
		WeaponMesh->SetAnimInstanceClass(nullptr);
	}

	UEstGameplayStatics::SetHighlightState(this, true);

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

	if (SoundManifest.Holster)
	{
		UGameplayStatics::SpawnSoundAttached(SoundManifest.Holster, WeaponMesh);
	}

	bIsHolstered = true;
}

void AEstBaseWeapon::Unholster()
{
	OnUnholster();

	if (SoundManifest.Unholster)
	{
		UGameplayStatics::SpawnSoundAttached(SoundManifest.Unholster, WeaponMesh);
	}

	bIsHolstered = false;
}