// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EstUIStatics.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API UEstUIStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void GetChildrenRecursively(class UUserWidget* UserWidget, TArray<class UWidget*>& Widgets);

	UFUNCTION(BlueprintCallable)
	static UWidget* GetFirstFocusableWidget(TArray<class UWidget*> Widgets);

	UFUNCTION(BlueprintCallable)
	static UWidget* GetFirstFocussedWidget(TArray<class UWidget*> Widgets);

	UFUNCTION(BlueprintCallable)
	static UScrollBox* GetFirstScrollBoxWidget(TArray<class UWidget*> Widgets);

	UFUNCTION(BlueprintPure)
	static bool IsInteractable(UWidget* Widget);
};
