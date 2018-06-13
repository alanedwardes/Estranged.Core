#pragma once

#include "GameFramework/Character.h"
#include "EstSaveRestore.h"
#include "EstBaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOnChangeWeaponDelegate, AEstBaseWeapon*, Weapon);

/** The base class for all of the characters in Estranged. */
UCLASS(abstract, BlueprintType)
class ESTCORE_API AEstBaseCharacter : public ACharacter, public IEstSaveRestore
{
	GENERATED_BODY()

public:
	AEstBaseCharacter(const class FObjectInitializer& ObjectInitializer);

// Begin IEstSaveRestore
	void OnPreRestore_Implementation() override {};
	void OnPostRestore_Implementation() override {};
	void OnPreSave_Implementation() override {};
	void OnPostSave_Implementation() override {};
// End IEstSaveRestore

	void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UEstHealthComponent *HealthComponent;

	/** The character's currently equipped weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pickup)
	TWeakObjectPtr<class AEstBaseWeapon> EquippedWeapon;

	/** Equip the specified weapon. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Munitions)
	void EquipWeapon(AEstBaseWeapon* Weapon);

	UPROPERTY(BlueprintAssignable, Category = Fighting)
	FOnOnChangeWeaponDelegate OnChangeWeapon;

	/** Throw down the equipped weapon */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Munitions)
	void UnequipWeapon();

	/** Does the player have a weapon */
	UFUNCTION(BlueprintPure, Category = "Fighting")
	virtual bool HasWeapon();

	/** Damage control */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	UFUNCTION()
	virtual bool CanJumpInternal_Implementation() const override;

	UFUNCTION()
	virtual bool CanCrouch() override;

	UFUNCTION()
	virtual bool CanSprint();

	UFUNCTION()
	class UEstCharacterMovementComponent* GetEstCharacterMovement() const { return EstCharacterMovement; };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UEstCharacterMovementComponent* EstCharacterMovement;

	/** Animation: Weapon primary fire */
	UFUNCTION()
	virtual void PlayPrimaryFire();

	/** Animation: Weapon secondary fire */
	UFUNCTION()
	virtual void PlaySecondaryFire();

	/** Animation: Weapon reload */
	UFUNCTION()
	virtual void PlayReload();
};
