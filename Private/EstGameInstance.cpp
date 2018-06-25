// Estranged is a trade mark of Alan Edwardes.

#include "EstCore.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "EstGameInstance.h"

void UEstGameInstance::Init()
{
	if (!IsDedicatedServerInstance())
	{
		UUserWidget* MenuUserWidget = CreateWidget<UUserWidget>(this, MenuWidgetType);
		MenuSlateWidget = MenuUserWidget->TakeWidget();
	}

	Super::Init();
}

void UEstGameInstance::SetMenuVisibleForever(bool bNewIsMenuVisibleForever)
{
	bIsMenuVisibleForever = bNewIsMenuVisibleForever;
	SetMenuVisibility(bNewIsMenuVisibleForever);
}

void UEstGameInstance::SetMenuVisibility(bool bNewIsMenuVisible)
{
	if (bNewIsMenuVisible)
	{
		GetWorld()->GetGameViewport()->AddViewportWidgetContent(MenuSlateWidget.ToSharedRef());
	}
	else if (!bIsMenuVisibleForever)
	{
		GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(MenuSlateWidget.ToSharedRef());
	}
}