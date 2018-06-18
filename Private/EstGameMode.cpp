// Estranged is a trade mark of Alan Edwardes.

#include "EstCore.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "EstGameMode.h"

void AEstGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MenuSlateWidget.IsValid() && GetWorld() && GetWorld()->GetGameViewport())
	{
		GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(MenuSlateWidget.ToSharedRef());
	}

	Super::EndPlay(EndPlayReason);
}

void AEstGameMode::SetMenuVisibility(bool bNewIsVisible)
{
	if (IsActorBeingDestroyed())
	{
		return;
	}

	if (bNewIsVisible)
	{
		if (!MenuSlateWidget.IsValid())
		{
			UUserWidget* MenuUserWidget = CreateWidget<UUserWidget>(GetGameInstance(), MenuWidgetType);
			MenuSlateWidget = MenuUserWidget->TakeWidget();
		}

		GetWorld()->GetGameViewport()->AddViewportWidgetContent(MenuSlateWidget.ToSharedRef());
	}
	else if (!bIsVisibleForever && MenuSlateWidget.IsValid())
	{
		GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(MenuSlateWidget.ToSharedRef());
	}
}