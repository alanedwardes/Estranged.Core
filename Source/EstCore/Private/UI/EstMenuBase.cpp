// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstMenuBase.h"
#include "UI/EstUIStatics.h"
#include "Blueprint/WidgetTree.h"
#include "Framework/Application/SlateApplication.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstMenuBase)

void UEstMenuBase::OnBack_Implementation()
{
}

FReply UEstMenuBase::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	UWidget* WidgetToFocus = LandingWidget;

	// If no explicit widget is set, find a candidate
	if (WidgetToFocus == nullptr)
	{
		TArray<UWidget*> Widgets;
		UEstUIStatics::GetChildrenRecursively(this, Widgets);
		WidgetToFocus = UEstUIStatics::GetFirstFocusableWidget(Widgets);
	}

	if (WidgetToFocus != nullptr)
	{
		TSharedPtr<SWidget> CapturingSlateWidget = WidgetToFocus->GetCachedWidget();
		if (CapturingSlateWidget.IsValid())
		{
			return FReply::Handled().SetUserFocus(CapturingSlateWidget.ToSharedRef(), InFocusEvent.GetCause(), false);
		}
	}

	return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

FReply UEstMenuBase::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Back)
	{
		OnBack();
		return FReply::Handled();
	}

	return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
}
