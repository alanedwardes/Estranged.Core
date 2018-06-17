#include "EstCore.h"
#include "EstSaveStatics.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "EstPlayerController.h"

void AEstPlayerController::Destroyed()
{
	SetMenuVisible(false);
	Super::Destroyed();
}

bool AEstPlayerController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	bIsUsingGamepad = bGamepad;
	return Super::InputKey(Key, EventType, AmountDepressed, bGamepad);
}

bool AEstPlayerController::InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	bIsUsingGamepad = bGamepad;
	return Super::InputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
}

void AEstPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UEstGameplaySave* GameplaySave = UEstSaveStatics::LoadGameplaySave();
	UEstSaveStatics::ApplyGameplaySave(GameplaySave, this);
}

void AEstPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (MenuWidgetType != nullptr)
	{
		MenuWidget = CreateWidget<UUserWidget>(this, MenuWidgetType);
		MenuSlateWidgetInstance = MenuWidget->TakeWidget();
	}

	SetMenuVisible(false);
}

void AEstPlayerController::SetMenuVisible(bool bNewIsVisible)
{
	if (!MenuSlateWidgetInstance.IsValid())
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		return;
	}

	UGameViewportClient* View = LocalPlayer->ViewportClient;
	if (View == nullptr)
	{
		return;
	}

	if (bNewIsVisible && !bIsMenuVisible)
	{
		MenuWidget->SetIsEnabled(true);
		View->AddViewportWidgetContent(MenuSlateWidgetInstance.ToSharedRef(), 1);

		FInputModeUIOnly InputMode = FInputModeUIOnly();
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetWidgetToFocus(MenuSlateWidgetInstance);
		SetInputMode(InputMode);

		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;

		bIsMenuVisible = true;
	}
	else
	{
		MenuWidget->SetIsEnabled(false);
		View->RemoveViewportWidgetContent(MenuSlateWidgetInstance.ToSharedRef());

		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;

		bIsMenuVisible = false;
	}
}