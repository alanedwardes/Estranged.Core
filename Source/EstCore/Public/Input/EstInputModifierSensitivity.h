// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "EstInputModifierSensitivity.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Estranged Mouse Sensitivity"))
class ESTCORE_API UEstInputModifierMouseSensitivity : public UInputModifier
{
	GENERATED_BODY()
	
protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

	UPROPERTY(Transient)
	class UEstEnhancedInputUserSettings* Settings;
};

UCLASS(NotBlueprintable, meta = (DisplayName = "Estranged Gamepad Sensitivity"))
class ESTCORE_API UEstInputModifierGamepadSensitivity : public UInputModifier
{
	GENERATED_BODY()

protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

	UPROPERTY(Transient)
	class UEstEnhancedInputUserSettings* Settings;
};
