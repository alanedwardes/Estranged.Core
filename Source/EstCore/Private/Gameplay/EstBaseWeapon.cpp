#include "Gameplay/EstBaseWeapon.h"
#include "EstCore.h"
#include "Engine/Engine.h"
#include "Gameplay/EstGameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Gameplay/EstBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"

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
}

void AEstBaseWeapon::OnPostRestore_Implementation()
{
	if (!OwnerSaveId.IsValid())
	{
		return;
	}

	AActor* FoundOwner = UEstGameplayStatics::FindActorBySaveIdInWorld(GetWorld(), OwnerSaveId);
	if (FoundOwner == nullptr)
	{
		return;
	}

	AEstBaseCharacter* Character = Cast<AEstBaseCharacter>(FoundOwner);
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
		WeaponMesh->SetUseCCD(false);
		SetActorEnableCollision(false);
	}
	else
	{
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetUseCCD(true);
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
	if (IsHolstered())
	{
		return;
	}

	bIsPrimaryFirePressed = true;
}

void AEstBaseWeapon::PrimaryAttackEnd()
{
	bIsPrimaryFirePressed = false;
}

void AEstBaseWeapon::SecondaryAttackStart()
{
	if (IsHolstered())
	{
		return;
	}

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

	if (ForceFeedbackManifest.PrimaryAttack && OwnerCharacter->IsPlayerControlled())
	{
		UEstGameplayStatics::SpawnForceFeedbackAttached(ForceFeedbackManifest.PrimaryAttack, WeaponMesh);
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

	if (ForceFeedbackManifest.SecondaryAttack && OwnerCharacter->IsPlayerControlled())
	{
		UEstGameplayStatics::SpawnForceFeedbackAttached(ForceFeedbackManifest.SecondaryAttack, WeaponMesh);
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

	OnPrimaryAttack.AddDynamic(NewOwnerCharacter, &AEstBaseCharacter::PlayPrimaryFire);
	OnSecondaryAttack.AddDynamic(NewOwnerCharacter, &AEstBaseCharacter::PlaySecondaryFire);

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

	OnPrimaryAttack.Clear();
	OnSecondaryAttack.Clear();

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
	if (IsEngagedInActivity())
	{
		return;
	}

	if (SoundManifest.Holster)
	{
		UGameplayStatics::SpawnSoundAttached(SoundManifest.Holster, WeaponMesh);
	}

	SetEngagedInActivity(HolsterLength);
	bIsHolstered = true;
	OnHolster();
}

void AEstBaseWeapon::Unholster()
{
	if (IsEngagedInActivity())
	{
		return;
	}

	if (SoundManifest.Unholster)
	{
		UGameplayStatics::SpawnSoundAttached(SoundManifest.Unholster, WeaponMesh);
	}

	SetEngagedInActivity(UnholsterLength);
	bIsHolstered = false;
	OnUnholster();
}