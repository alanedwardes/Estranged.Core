#include "Gameplay/EstPlayerController.h"
#include "EstCore.h"
#include "Gameplay/EstSaveStatics.h"
#include "Gameplay/EstGameInstance.h"
#include "Gameplay/EstPlayerCameraManager.h"

AEstPlayerController::AEstPlayerController(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PlayerCameraManagerClass = AEstPlayerCameraManager::StaticClass();
}

bool AEstPlayerController::InputKey(const FInputKeyParams& Params)
{
	bIsUsingGamepad = Params.IsGamepad();
	return Super::InputKey(Params);
}

void AEstPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Menu", IE_Released, this, &AEstPlayerController::ShowMainMenu);
	InputComponent->BindAction("Console", IE_Released, this, &AEstPlayerController::ShowConsoleMenu);
}

void AEstPlayerController::ShowMenuSection(FName MenuSection)
{
	SetPause(true);
	UEstGameplayStatics::GetEstGameInstance(this)->SetMenuVisibility(FEstMenuVisibilityContext(true, MenuSection));
	SetMenuFocusState(true);
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