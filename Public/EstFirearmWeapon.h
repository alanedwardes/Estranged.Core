#pragma once

#include "EstBaseWeapon.h"
#include "EstFirearmWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadDelegate);

/** A base for all firearms in Estranged */
UCLASS(abstract, BlueprintType)
class ESTCORE_API AEstFirearmWeapon : public AEstBaseWeapon
{
	GENERATED_BODY()

public:
	AEstFirearmWeapon(const class FObjectInitializer& PCIP);

	/** Inherited primary attack */
	virtual void PrimaryAttack() override;
	/** Inherited secondary attack */
	virtual void SecondaryAttack() override;

	/** Inherited primary attack end */
	virtual void PrimaryAttackEnd() override;

	/** Weapon's reload */
	UFUNCTION()
	virtual void Reload();

	/** When reload starts */
	UPROPERTY(BlueprintAssignable, Category = Fighting)
	FOnReloadDelegate OnReload;

	/** Queue the weapon's reload. */
	UFUNCTION(BlueprintCallable, Category = Fighting)
	virtual void ReloadPressed();

	/** Tick to check if a reload is queued */
	virtual void Tick(float DeltaTime) override;

	virtual void PrimaryFireEffects();

	/** Skeletal mesh for the current weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fighting")
	class USpotLightComponent* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fighting)
	class TSubclassOf<AActor> AmunitionClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fighting)
	class TSubclassOf<class UCameraShake> PrimaryRecoilShake;

	/** Primary ammunition */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fighting)
	class UEstFirearmAmunition* PrimaryAmmunition;

	/** CONFIG - Enables secondary amunition for the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fighting, DisplayName = "Secondary Amunition")
	bool bHasSecondaryAmunition;

	/** Secondary ammunition */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fighting, meta = (EditCondition = "bHasSecondaryAmunition"))
	class UEstFirearmAmunition* SecondaryAmmunition;

	/** CONFIG - Projectile spread characteristics. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Fighting)
	float ProjectileSpread;

	/** CONFIG - Enables semi-automatic mode for the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fighting, DisplayName = "Semi-Automatic")
	bool bIsSemiAutomatic;

	/** STATE - The expiry time of an issued reload command */
	UPROPERTY(BlueprintReadOnly, Category = Fighting)
	float PrimaryFireTime;

	/** STATE - The expiry time of an issued reload command */
	UPROPERTY(BlueprintReadOnly, Category = Fighting)
	float ReloadExpireTime;

	/** Is the weapon currently engaged in an activity (reload, shoot etc) */
	UFUNCTION(BlueprintPure, Category = Fighting)
	virtual bool IsReloading();

	/** STATE - Semi-automatic mode blocking variable */
	UPROPERTY()
	bool bSemiBlocking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float ReloadLength;
};
