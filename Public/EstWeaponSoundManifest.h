#pragma once

#include "EstWeaponSoundManifest.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FEstWeaponSoundManifest
{
	GENERATED_USTRUCT_BODY()

		FEstWeaponSoundManifest()
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class USoundBase* PrimaryAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class USoundBase* SecondaryAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class USoundBase* Holster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class USoundBase* Unholster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Firearm)
	class USoundBase* Reload;
};