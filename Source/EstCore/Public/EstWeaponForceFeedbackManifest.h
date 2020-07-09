#pragma once

#include "EstWeaponForceFeedbackManifest.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FEstWeaponForceFeedbackManifest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class UForceFeedbackEffect* PrimaryAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class UForceFeedbackEffect* SecondaryAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Firearm)
	class UForceFeedbackEffect* Reload;
};