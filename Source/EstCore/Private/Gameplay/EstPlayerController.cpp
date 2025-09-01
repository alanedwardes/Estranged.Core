#include "Gameplay/EstPlayerController.h"
#include "EstCore.h"
#include "UI/EstHUDWidget.h"
#include "Gameplay/EstSaveStatics.h"
#include "Gameplay/EstGameInstance.h"
#include "Gameplay/EstPlayerCameraManager.h"
#include "GameFramework/PlayerInput.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AEstPlayerController::AEstPlayerController(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PlayerCameraManagerClass = AEstPlayerCameraManager::StaticClass();
}

bool AEstPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	bIsUsingGamepad = Params.IsGamepad();
	return Super::InputKey(Params);
}

void AEstPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		FModifyContextOptions ModifyContextOptions;
		ModifyContextOptions.bNotifyUserSettings = true;
		Subsystem->AddMappingContext(FirstPersonMappingContext, 0, ModifyContextOptions);
		Subsystem->AddMappingContext(MainMappingContext, INT32_MAX);
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		// Menu
		EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Started, this, &AEstPlayerController::ShowMainMenu);

		// Console
		EnhancedInputComponent->BindAction(ConsoleAction, ETriggerEvent::Started, this, &AEstPlayerController::ShowConsoleMenu);
	}
}

void AEstPlayerController::ShowMenuSection(FName MenuSection)
{
	SetPause(true);
	UEstGameplayStatics::GetEstGameInstance(this)->SetMenuVisibility(FEstMenuVisibilityContext(true, MenuSection));
	SetMenuFocusState(true);
}

void AEstPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!HUDWidgetClass.IsNull())
	{
		HUDWidget = CreateWidget<UEstHUDWidget>(this, HUDWidgetClass.LoadSynchronous());
		HUDWidget->AddToViewport(0);
	}
}

void AEstPlayerController::BeginDestroy()
{
	Super::BeginDestroy();

	if (HUDWidget != nullptr)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

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