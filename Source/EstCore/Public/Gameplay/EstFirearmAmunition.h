#pragma once

#include "Interfaces/EstSaveRestore.h"
#include "EstFirearmAmunition.generated.h"

/** A base for all firearm ammunition in Estranged */
UCLASS()
class ESTCORE_API UEstFirearmAmunition : public UActorComponent, public IEstSaveRestore
{
	GENERATED_BODY()

// Begin IEstSaveRestore
	void OnPreRestore_Implementation() override {};
	void OnPostRestore_Implementation() override {};
	void OnPreSave_Implementation() override {};
	void OnPostSave_Implementation() override {};
// End IEstSaveRestore

public:

	/** Total ammo remanining */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Ammo)
	int32 AmmoRemaining;
	/** Maximum ammo this gun can carry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo)
	int32 AmmoMax;
	/** Total clip remaining */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = Clip)
	int32 ClipRemaining;
	/** Maximum ammo the clip can carry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Clip)
	int32 ClipMax;

	/** AI can have infinite clips */
	UFUNCTION(BlueprintPure, Category = Clip)
	bool HasInfiniteClips()
	{
		if (!GetOwner())
		{
			return false;
		}

		APawn* Pawn = Cast<APawn>(GetOwner()->GetOwner());
		if (!Pawn)
		{
			return false;
		}

		return !Pawn->IsPlayerControlled();
	}

	/** Can we fire, given the current ammunition state */
	UFUNCTION(BlueprintPure, Category = Clip)
	bool CanFire()
	{
		return ClipRemaining > 0;
	}

	UFUNCTION(BlueprintCallable, Category = Ammo)
	void AddAmmo(int32 NewAmmo)
	{
		if (AmmoRemaining < AmmoMax)
		{
			AmmoRemaining = FMath::Min(AmmoMax, AmmoRemaining + NewAmmo);
			return;
		}
	}

	UFUNCTION(BlueprintPure, Category = Ammo)
	bool CanAddAmmo()
	{
		if (AmmoRemaining < AmmoMax)
		{
			return true;
		}

		return false;
	}

	/** Fire a round */
	UFUNCTION(BlueprintCallable, Category = Clip)
	void Fire()
	{
		ensure(CanFire());
		ClipRemaining--;
	}

	/** Can we reload, given the current magazine and loaded round state */
	UFUNCTION(BlueprintPure, Category = Ammo)
	bool CanReload()
	{
		if (HasInfiniteClips())
		{
			return true;
		}

		return AmmoRemaining > 0 && ClipRemaining < ClipMax;
	}

	/** Reload */
	UFUNCTION(BlueprintCallable, Category = Ammo)
	void Reload()
	{
		ensure(CanReload());

		if (HasInfiniteClips())
		{
			ClipRemaining = ClipMax;
			return;
		}

		int32 AmmoToRefil = ClipMax - ClipRemaining;
		if (AmmoToRefil > AmmoRemaining)
		{
			ClipRemaining += AmmoRemaining;
			AmmoRemaining = 0;
		}
		else
		{
			ClipRemaining = ClipMax;
			AmmoRemaining -= AmmoToRefil;
		}
	}
};
