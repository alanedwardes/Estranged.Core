// Estranged is a trade mark of Alan Edwardes.

#include "Input/EstInputModifierSensitivity.h"
#include "EnhancedPlayerInput.h"
#include "EnhancedInputSubsystems.h"
#include "Input/EstEnhancedInputUserSettings.h"

FInputActionValue UEstInputModifierMouseSensitivity::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	if (!Settings)
	{
		Settings = PlayerInput->GetOwningLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->GetUserSettings<UEstEnhancedInputUserSettings>();
	}

	return CurrentValue.Get<FVector>() * Settings->GetSensitivityMouse();
}

FInputActionValue UEstInputModifierGamepadSensitivity::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	if (!Settings)
	{
		Settings = PlayerInput->GetOwningLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->GetUserSettings<UEstEnhancedInputUserSettings>();
	}

	return CurrentValue.Get<FVector>() * Settings->GetSensitivityGamepad();
}