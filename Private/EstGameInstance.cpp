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

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEstGameInstance::PreLoadMap);

	Super::Init();
}

void UEstGameInstance::PreLoadMap(const FString & InMapName)
{
	SetMenuVisibleForever(false);
}

void UEstGameInstance::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);

	MenuSlateWidget = nullptr;

	Super::Shutdown();
}

void UEstGameInstance::SetMenuVisibleForever(bool bNewIsMenuVisibleForever)
{
	bIsMenuVisibleForever = bNewIsMenuVisibleForever;
	SetMenuVisibility(bNewIsMenuVisibleForever);
}

void UEstGameInstance::SetMenuVisibility(bool bNewIsMenuVisible)
{
	if (!MenuSlateWidget.IsValid())
	{
		return;
	}

	if (bNewIsMenuVisible)
	{
		GetWorld()->GetGameViewport()->AddViewportWidgetContent(MenuSlateWidget.ToSharedRef());

		// Forcefully enable the mouse cursor at the platform level. This works around a bug in the Ansel plugin
		TSharedPtr<GenericApplication> PlatformApplication = FSlateApplicationBase::Get().GetPlatformApplication();
		if (PlatformApplication.IsValid() && PlatformApplication->Cursor.IsValid())
		{
			PlatformApplication->Cursor->Show(true);
		}
	}
	else if (!bIsMenuVisibleForever)
	{
		GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(MenuSlateWidget.ToSharedRef());
	}
}