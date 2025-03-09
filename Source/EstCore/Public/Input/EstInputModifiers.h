// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "EstInputModifiers.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Estranged Mouse Sensitivity"))
class ESTCORE_API UEstInputModifierMouse : public UInputModifier
{
	GENERATED_BODY()
	
protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

	UPROPERTY(Transient)
	class UEstEnhancedInputUserSettings* Settings;
};

UCLASS(NotBlueprintable, meta = (DisplayName = "Estranged Gamepad Sensitivity"))
class ESTCORE_API UEstInputModifierGamepad : public UInputModifier
{
	GENERATED_BODY()

protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

	UPROPERTY(Transient)
	class UEstEnhancedInputUserSettings* Settings;
};
