#include "EstPlayerController.h"
#include "EstCore.h"
#include "EstSaveStatics.h"
#include "EstGameInstance.h"
#include "EstPlayerCameraManager.h"

AEstPlayerController::AEstPlayerController(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PlayerCameraManagerClass = AEstPlayerCameraManager::StaticClass();
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

void AEstPlayerController::BeginDestroy()
{
	Super::BeginDestroy();

	SetMenuFocusState(false);
}

void AEstPlayerController::SetMenuFocusState(bool bNewIsFocussed)
{
	if (bNewIsFocussed)
	{
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;

		FInputModeUIOnly InputMode = FInputModeUIOnly();
		InputMode.SetWidgetToFocus(UEstGameplayStatics::GetEstGameInstance(this)->GetRawMenuWidget());
		SetInputMode(InputMode);
	}
	else
	{
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;

		SetInputMode(FInputModeGameOnly());
	}
}