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

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Sensitivity")
	float SensitivityMouse;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Sensitivity")
	float SensitivityGamepad;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Inversion")
	bool InvertMouseY;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Inversion")
	bool InvertGamepadY;
};
