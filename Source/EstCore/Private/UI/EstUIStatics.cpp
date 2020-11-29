// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstUIStatics.h"
#include "Blueprint/UserWidget.h"

void UEstUIStatics::GetChildrenRecursively(UUserWidget* UserWidget, TArray<UWidget*>& Widgets)
{
	if (UserWidget == nullptr)
	{
		return;
	}

	UserWidget->WidgetTree->GetAllWidgets(Widgets);

	for (UWidget* Widget : TArray<UWidget*>(Widgets))
	{
		if (UUserWidget* ChildUserWidget = Cast<UUserWidget>(Widget))
		{
			ChildUserWidget->WidgetTree->GetAllWidgets(Widgets);
		}
	}
}

UWidget* UEstUIStatics::GetFirstFocusableWidget(TArray<UWidget*> Widgets)
{
	for (UWidget* Widget : Widgets)
	{
		if (!UEstUIStatics::IsInteractable(Widget))
		{
			continue;
		}

		if (UUserWidget* UserWidget = Cast<UUserWidget>(Widget))
		{
			if (UserWidget->bIsFocusable)
			{
				return Widget;
			}
		}
	}

	return nullptr;
}

UWidget* UEstUIStatics::GetFirstFocussedWidget(TArray<UWidget*> Widgets)
{
	for (UWidget* Widget : Widgets)
	{
		if (!UEstUIStatics::IsInteractable(Widget))
		{
			continue;
		}

		if (Widget->HasAnyUserFocus())
		{
			return Widget;
		}
	}

	return nullptr;
}

UScrollBox* UEstUIStatics::GetFirstScrollBoxWidget(TArray<UWidget*> Widgets)
{
	for (UWidget* Widget : Widgets)
	{
		if (!UEstUIStatics::IsInteractable(Widget))
		{
			continue;
		}

		if (UScrollBox* ScrollBox = Cast<UScrollBox>(Widget))
		{
			return ScrollBox;
		}
	}

	return nullptr;
}

bool UEstUIStatics::IsInteractable(UWidget* Widget)
{
	if (Widget == nullptr)
	{
		return false;
	}

	if (!Widget->GetIsEnabled())
	{
		return false;
	}

	if (Widget->GetVisibility() == ESlateVisibility::Hidden || 
		Widget->GetVisibility() == ESlateVisibility::Collapsed ||
		Widget->GetVisibility() == ESlateVisibility::HitTestInvisible)
	{
		return false;
	}

	return true;
}
