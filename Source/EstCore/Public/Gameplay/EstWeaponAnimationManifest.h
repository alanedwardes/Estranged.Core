#pragma once

#include "EstWeaponAnimationManifest.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FEstWeaponAnimationManifest
{
	GENERATED_BODY()

	FEstWeaponAnimationManifest() : PrimaryAttack(nullptr), SecondaryAttack(nullptr), Reload(nullptr) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class UAnimMontage* PrimaryAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class UAnimMontage* SecondaryAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Firearm)
	class UAnimMontage* Reload;
};