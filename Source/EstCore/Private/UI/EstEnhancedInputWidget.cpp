// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstEnhancedInputWidget.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstEnhancedInputWidget)

bool UEstEnhancedInputWidget::TryFireActionForKey(const FKey& Key)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)
	{
		return false;
	}

	for (UInputAction* Action : BoundActions)
	{
		if (Subsystem->QueryKeysMappedToAction(Action).Contains(Key))
		{
			OnInputActionTriggered(Action, FInputActionValue(true));
			return true;
		}
	}

	return false;
}

FReply UEstEnhancedInputWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (TryFireActionForKey(InKeyEvent.GetKey()))
	{
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UEstEnhancedInputWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (TryFireActionForKey(InMouseEvent.GetEffectingButton()))
	{
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
