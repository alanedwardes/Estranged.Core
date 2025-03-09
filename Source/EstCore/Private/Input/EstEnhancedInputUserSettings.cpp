// Estranged is a trade mark of Alan Edwardes.


#include "Input/EstEnhancedInputUserSettings.h"

UEstEnhancedInputUserSettings::UEstEnhancedInputUserSettings()
{
	SensitivityMouse = FVector::OneVector;
	SensitivityGamepad = FVector::OneVector;
}

void UEstEnhancedInputUserSettings::SetSensitivtyMouse(FVector NewSensitivityMouse)
{
	SensitivityMouse = NewSensitivityMouse;
}

void UEstEnhancedInputUserSettings::SetSensitivtyGamepad(FVector NewSensitivityGamepad)
{
	SensitivityGamepad = NewSensitivityGamepad;
}

FVector UEstEnhancedInputUserSettings::GetSensitivityMouse() const
{
	return SensitivityMouse;
}

FVector UEstEnhancedInputUserSettings::GetSensitivityGamepad() const
{
	return SensitivityGamepad;
}
