// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "EstEnhancedInputUserSettings.generated.h"

UCLASS()
class ESTCORE_API UEstEnhancedInputUserSettings : public UEnhancedInputUserSettings
{
	GENERATED_BODY()
	
public:
	UEstEnhancedInputUserSettings();

	UFUNCTION(BlueprintCallable)
	void SetSensitivtyMouse(FVector NewSensitivityMouse);

	UFUNCTION(BlueprintCallable)
	void SetSensitivtyGamepad(FVector NewSensitivityGamepad);

	UFUNCTION(BlueprintPure)
	FVector GetSensitivityMouse() const;

	UFUNCTION(BlueprintPure)
	FVector GetSensitivityGamepad() const;

private:
	FVector SensitivityMouse;
	FVector SensitivityGamepad;
};
