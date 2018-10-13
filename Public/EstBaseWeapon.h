#pragma once

#include "GameFramework/Actor.h"
#include "Object.h"
#include "EstInteractive.h"
#include "EstSaveRestore.h"
#include "EstWeaponAnimationManifest.h"
#include "EstWeaponSoundManifest.h"
#include "EstBaseWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquippedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPrimaryAttackDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSecondaryAttackDelegate);

/** The base for all of the weapons in Estranged. */
UCLASS(abstract, BlueprintType)
class ESTCORE_API AEstBaseWeapon : public AActor, public IEstInteractive, public IEstSaveRestore
{
	GENERATED_BODY()

public:
	AEstBaseWeapon(const class FObjectInitializer& PCIP);

// Begin IEstSaveRestore
	void OnPreRestore_Implementation() override {};
	void OnPostRestore_Implementation() override;
	void OnPreSave_Implementation() override;
	void OnPostSave_Implementation() override {};
// End IEstSaveRestore

	/** Register all components for this actor. */
	virtual void PostRegisterAllComponents() override;

	/** Respond to the native C++ use event. */
	virtual bool OnUsed_Implementation(AEstBaseCharacter* User, class USceneComponent* UsedComponent) override;

	/** Respond to the native C++ use event. */
	virtual void OnUnused_Implementation(AEstBaseCharacter* User) override { };

	/** Tick to check if primary/secondary attack is pressed */
	virtual void Tick(float DeltaTime) override;

	/** Create either the player skeleton or the physical mesh, 
		depending on whether the weapon is equipped or not */
	UFUNCTION()
	virtual void UpdateWeaponPhysicsState();

	/** Equipping the weapon */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual void Equip(class AEstBaseCharacter* NewOwnerCharacter);
	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void OnEquip();
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual void Unequip();
	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void OnUnequip();
	UFUNCTION(BlueprintPure, Category = Inventory)
	virtual bool HasSecondaryAttack() { return false; };
	/** Do we currently have a character that we're equipped to */
	UFUNCTION(BlueprintPure, Category = Inventory)
	virtual bool IsEquipped();
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnEquippedDelegate OnEquipped;

	/** Holstering the weapon */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual void Holster();
	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void OnHolster();
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual void Unholster();
	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void OnUnholster();
	UFUNCTION(BlueprintPure, Category = Inventory)
	virtual bool IsHolstered() { return bIsHolstered; };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory)
	FName SocketName;

	/** Internal func to call for primary attack */
	UFUNCTION()
	virtual void PrimaryAttack();
	/** The event to implement in blueprints */
	UPROPERTY(BlueprintAssignable, Category = Fighting)
	FOnPrimaryAttackDelegate OnPrimaryAttack;
	/** Start the weapon's primary attack. */
	UFUNCTION(BlueprintCallable, Category = Fighting)
	virtual void PrimaryAttackStart();
	/** End the weapon's primary attack. */
	UFUNCTION(BlueprintCallable, Category = Fighting)
	virtual void PrimaryAttackEnd();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fighting)
	class TSubclassOf<class UCameraShake> PrimaryAttackShake;

	/** Internal func to call for secondary attack */
	UFUNCTION()
	virtual void SecondaryAttack();
	/** The event to implement in blueprints */
	UPROPERTY(BlueprintAssignable, Category = Fighting)
	FOnSecondaryAttackDelegate OnSecondaryAttack;
	/** Start the weapon's secondary attack. */
	UFUNCTION(BlueprintCallable, Category = Fighting)
	virtual void SecondaryAttackStart();
	/** End the weapon's secondary attack. */
	UFUNCTION(BlueprintCallable, Category = Fighting)
	virtual void SecondaryAttackEnd();

	/** Is the weapon currently engaged in an activity (reload, shoot etc) */
	UFUNCTION(BlueprintPure, Category = Fighting)
	virtual bool IsEngagedInActivity() { return GetWorld()->TimeSince(LastActivityFinishTime) < 0.f; };
	/** Set the length of time the weapon will be engaged in an activity for */
	UFUNCTION(BlueprintCallable, Category = Fighting)
	virtual void SetEngagedInActivity(float ActivityLength);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Player)
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	class TSubclassOf<UAnimInstance> EquippedWeaponAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	class TSubclassOf<UAnimInstance> PlayerAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	struct FEstWeaponAnimationManifest PlayerAnimManifest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	struct FEstWeaponAnimationManifest CharacterAnimManifest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Effects)
	struct FEstWeaponSoundManifest SoundManifest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	struct FEstWeaponAnimationManifest WeaponAnimManifest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	FTransform NormalViewModelOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation, meta = (EditCondition = "bCanAim"))
	FTransform AimingViewModelOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	bool bCanAim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Handling)
	float PrimaryAttackLengthPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Handling)
	float PrimaryAttackLengthAI;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Handling)
	float SecondaryAttackLengthPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Handling)
	float SecondaryAttackLengthAI;

	/** STATE - Should we continue to fire the primary ammunition */
	UPROPERTY(BlueprintReadOnly, Category = Fighting)
	bool bIsPrimaryFirePressed;

	/** STATE - Should we continue to fire the secondary ammunition */
	UPROPERTY(BlueprintReadOnly, Category = Fighting)
	bool bIsSecondaryFirePressed;

	/** STATE - Is the weapon in its holster */
	UPROPERTY(BlueprintReadOnly, Category = Fighting)
	bool bIsHolstered;

	/** STATE - Is the weapon equipped by AI or a player */
	UPROPERTY(BlueprintReadOnly, Category = Fighting)
	class AEstBaseCharacter* OwnerCharacter;

	/** STATE, INTERNAL - world time in seconds of the end of the last activity */
	UPROPERTY(BlueprintReadOnly, Category = Fighting)
	float LastActivityFinishTime;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category = Fighting)
	FName OwnerActorName;
};
