#pragma once

#include "EstWeaponForceFeedbackManifest.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FEstWeaponForceFeedbackManifest
{
	GENERATED_BODY()

	FEstWeaponForceFeedbackManifest() : PrimaryAttack(nullptr), SecondaryAttack(nullptr), Reload(nullptr) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class UForceFeedbackEffect* PrimaryAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class UForceFeedbackEffect* SecondaryAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Firearm)
	class UForceFeedbackEffect* Reload;
};