// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EstMenuWidget.generated.h"

UCLASS(abstract)
class ESTCORE_API UEstMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnShowMenu(FName RedirectToMenu);
};
