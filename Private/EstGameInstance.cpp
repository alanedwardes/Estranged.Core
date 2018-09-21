// Estranged is a trade mark of Alan Edwardes.

#include "EstCore.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "EstMenuWidget.h"
#include "EstGameInstance.h"

void UEstGameInstance::Init()
{
	if (!IsDedicatedServerInstance())
	{
		MenuUserWidget = CreateWidget<UEstMenuWidget>(this, MenuWidgetType);
		MenuSlateWidget = MenuUserWidget->TakeWidget();
	}

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEstGameInstance::PreLoadMap);

	Super::Init();
}

void UEstGameInstance::PreLoadMap(const FString & InMapName)
{
	SetMenuVisibility(FEstMenuVisibilityContext(false, true));
}

void UEstGameInstance::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);

	MenuSlateWidget = nullptr;

	Super::Shutdown();
}

void UEstGameInstance::SetMenuVisibility(FEstMenuVisibilityContext InVisibilityContext)
{
	if (!MenuSlateWidget.IsValid())
	{
		return;
	}

	VisibilityContext = InVisibilityContext;
	if (VisibilityContext.bIsMenuVisible)
	{
		MenuUserWidget->AddToViewport();
		MenuUserWidget->OnShowMenu(InVisibilityContext.RedirectToMenu);

		// Forcefully enable the mouse cursor at the platform level. This works around a bug in the Ansel plugin
		TSharedPtr<GenericApplication> PlatformApplication = FSlateApplicationBase::Get().GetPlatformApplication();
		if (PlatformApplication.IsValid() && PlatformApplication->Cursor.IsValid())
		{
			PlatformApplication->Cursor->Show(true);
		}
	}
	else if (!VisibilityContext.bIsMenuVisibleForever)
	{
		MenuUserWidget->RemoveFromViewport();
	}
}