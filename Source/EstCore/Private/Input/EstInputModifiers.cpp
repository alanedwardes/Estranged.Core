// Estranged is a trade mark of Alan Edwardes.

#include "Input/EstInputModifiers.h"
#include "EnhancedPlayerInput.h"
#include "EnhancedInputSubsystems.h"
#include "Input/EstEnhancedInputUserSettings.h"

FInputActionValue UEstInputModifierMouse::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	if (!Settings)
	{
		Settings = PlayerInput->GetOwningLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->GetUserSettings<UEstEnhancedInputUserSettings>();
	}

	FVector Multiplier = FVector(Settings->SensitivityMouse, Settings->InvertMouseY ? -Settings->SensitivityMouse : Settings->SensitivityMouse, 0.f);

	return CurrentValue.Get<FVector>() * Multiplier;
}

FInputActionValue UEstInputModifierGamepad::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	if (!Settings)
	{
		Settings = PlayerInput->GetOwningLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->GetUserSettings<UEstEnhancedInputUserSettings>();
	}

	FVector Multiplier = FVector(Settings->SensitivityGamepad, Settings->InvertGamepadY ? -Settings->SensitivityGamepad : Settings->SensitivityGamepad, 0.f);

	return CurrentValue.Get<FVector>() * Multiplier;
}
