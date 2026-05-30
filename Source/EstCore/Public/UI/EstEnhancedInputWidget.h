// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "Blueprint/UserWidget.h"
#include "EstEnhancedInputWidget.generated.h"

class UInputAction;
struct FInputActionValue;

UCLASS(Abstract)
class ESTCORE_API UEstEnhancedInputWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TArray<TObjectPtr<UInputAction>> BoundActions;

	UFUNCTION(BlueprintImplementableEvent)
	void OnInputActionTriggered(UInputAction* Action, const FInputActionValue& Value);

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	bool TryFireActionForKey(const FKey& Key);
};
