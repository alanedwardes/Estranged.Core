// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstMenuWidget.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PanelWidget.h"
#include "Components/OverlaySlot.h"
#include "Gameplay/EstGameInstance.h"
#include "Gameplay/EstGameplayStatics.h"
#include "Gameplay/EstPlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "EnhancedInputSubsystems.h"
#include "EstCore.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstMenuWidget)

void UEstMenuWidget::Action(FEstMenuAction MenuAction)
{
	switch (MenuAction.Action)
	{
	case EEstMenuAction::ResumeGame:
		OnResumeGame();
		break;
	case EEstMenuAction::ExitGame:
		FGenericPlatformMisc::RequestExit(false);
		break;
	case EEstMenuAction::TransitionToLevel:
		OnLoadLevelByReference(MenuAction.Level);
		break;
	case EEstMenuAction::TransitionToCheckpoint:
		OnLoadCheckpoint(MenuAction.Checkpoint);
		break;
	case EEstMenuAction::ReloadCurrentLevel:
		if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
		{
			FEstMenuAction NewMenuAction;
			NewMenuAction.Action = EEstMenuAction::TransitionToLevel;
			NewMenuAction.Level = World;
			Action(NewMenuAction);
		}
		break;
	}
}

void UEstMenuWidget::AsyncNavigate(TSoftClassPtr<UEstMenuSection> MenuSection, FName Context)
{
	RemoveMenu();

	TWeakObjectPtr<UEstMenuWidget> WeakThis(this);

	FSoftObjectPath StreamingObjectPath = MenuSection.ToSoftObjectPath();
	TSharedPtr<FStreamableHandle> StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(StreamingObjectPath, [WeakThis, Context, MenuSection]() {
		if (UEstMenuWidget* StrongThis = WeakThis.Get())
		{
			if (!MenuSection.IsValid())
			{
				return;
			}

			UEstMenuSection* Widget = NewObject<UEstMenuSection>(StrongThis, MenuSection.Get(), NAME_None, RF_Transactional);
			Widget->SetPlayerContext(FLocalPlayerContext(StrongThis->GetOwningLocalPlayer(), StrongThis->GetWorld()));
			Widget->Context = Context;
			Widget->Initialize();
			StrongThis->Navigate(Widget, Context);
		}
	},
	FStreamableManager::AsyncLoadHighPriority);
}

void UEstMenuWidget::Navigate(UEstMenuSection* MenuSection, FName Context)
{
	if (MenuSectionContainer == nullptr)
	{
		return;
	}

	CurrentMenuSection = MenuSection;
	CurrentMenuSection->OnNavigate.AddDynamic(this, &UEstMenuWidget::AsyncNavigate);
	CurrentMenuSection->OnAction.AddDynamic(this, &UEstMenuWidget::Action);
	CurrentMenuSection->OnModal.AddDynamic(this, &UEstMenuWidget::AsyncModal);
	MenuSectionContainer->AddChild(CurrentMenuSection);

	if (CurrentMenuModal == nullptr)
	{
		SetAreaIsEnabled(EEstMenuArea::Section, true);
		FocusArea(EEstMenuArea::Section);
	}

	OnMenuLoadingStateChanged(false);
}

void UEstMenuWidget::RemoveMenu()
{
	if (CurrentMenuSection != nullptr)
	{
		UGameplayStatics::PlaySound2D(this, NavigationSound);
		CurrentMenuSection->OnNavigate.RemoveAll(this);
		CurrentMenuSection->OnAction.RemoveAll(this);
		CurrentMenuSection->OnModal.RemoveAll(this);
		MenuSectionContainer->RemoveChild(CurrentMenuSection);
		CurrentMenuSection = nullptr;

		OnMenuLoadingStateChanged(true);
	}
}

void UEstMenuWidget::FocusArea(EEstMenuArea Area)
{
	UUserWidget* WidgetToFocus = GetAreaWidget(Area);
	if (WidgetToFocus == nullptr)
	{
		return;
	}

	WidgetToFocus->SetUserFocus(GetOwningPlayer());
}

void UEstMenuWidget::SetAreaIsEnabled(EEstMenuArea Area, bool bNewIsEnabled)
{
	UUserWidget* WidgetToEnable = GetAreaWidget(Area);
	if (WidgetToEnable == nullptr)
	{
		return;
	}

	WidgetToEnable->SetIsEnabled(bNewIsEnabled);
}

UUserWidget* UEstMenuWidget::GetAreaWidget(EEstMenuArea Area)
{
	if (Area == EEstMenuArea::Modal)
	{
		return CurrentMenuModal;
	}
	else if (Area == EEstMenuArea::Extra)
	{
		return CurrentExtraSection;
	}
	else if (Area == EEstMenuArea::Section)
	{
		return CurrentMenuSection;
	}
	return nullptr;
}

void UEstMenuWidget::AsyncModal(TSoftClassPtr<UEstMenuModal> MenuModal, FName Context)
{
	TWeakObjectPtr<UEstMenuWidget> WeakThis(this);

	// Disable all other areas while the modal is loading
	SetAreaIsEnabled(EEstMenuArea::Section, false);
	SetAreaIsEnabled(EEstMenuArea::Extra, false);
	SetAreaIsEnabled(EEstMenuArea::Modal, false);

	FSoftObjectPath StreamingObjectPath = MenuModal.ToSoftObjectPath();
	TSharedPtr<FStreamableHandle> StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(StreamingObjectPath, [WeakThis, Context, MenuModal]() {
		if (UEstMenuWidget* StrongThis = WeakThis.Get())
		{
			if (!MenuModal.IsValid())
			{
				return;
			}

			UEstMenuModal* Widget = NewObject<UEstMenuModal>(StrongThis, MenuModal.Get(), NAME_None, RF_Transactional);
			Widget->SetPlayerContext(FLocalPlayerContext(StrongThis->GetOwningLocalPlayer(), StrongThis->GetWorld()));
			Widget->Context = Context;
Widget->Initialize();
StrongThis->Modal(Widget, Context);
		}
	},
	FStreamableManager::AsyncLoadHighPriority);
}

void UEstMenuWidget::Modal(UEstMenuModal* MenuModal, FName Context)
{
	if (MenuModalContainer == nullptr)
	{
		return;
	}

	RemoveModal();

	SetAreaIsEnabled(EEstMenuArea::Section, false);
	SetAreaIsEnabled(EEstMenuArea::Extra, false);
	SetAreaIsEnabled(EEstMenuArea::Modal, true);
	FocusArea(EEstMenuArea::Modal);

	CurrentMenuModal = MenuModal;
	CurrentMenuModal->Context = Context;
	CurrentMenuModal->OnExit.AddDynamic(this, &UEstMenuWidget::ExitModal);
	UOverlaySlot* OverlaySlot = MenuModalContainer->AddChildToOverlay(CurrentMenuModal);
	OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
	OverlaySlot->SetVerticalAlignment(VAlign_Fill);
	CurrentMenuModal->SetUserFocus(GetOwningPlayer());
}

void UEstMenuWidget::RemoveModal()
{
	if (CurrentMenuModal != nullptr)
	{
		UGameplayStatics::PlaySound2D(this, NavigationSound);
		CurrentMenuModal->OnExit.RemoveAll(this);
		MenuModalContainer->RemoveChild(CurrentMenuModal);
		CurrentMenuModal = nullptr;
	}
}

void UEstMenuWidget::ExitModal()
{
	RemoveModal();

	SetAreaIsEnabled(EEstMenuArea::Modal, false);
	SetAreaIsEnabled(EEstMenuArea::Extra, true);
	SetAreaIsEnabled(EEstMenuArea::Section, true);
	FocusArea(EEstMenuArea::Section);
}

void UEstMenuWidget::AsyncExtra(TSoftClassPtr<UUserWidget> ExtraSection)
{
	TWeakObjectPtr<UEstMenuWidget> WeakThis(this);

	FSoftObjectPath StreamingObjectPath = ExtraSection.ToSoftObjectPath();
	TSharedPtr<FStreamableHandle> StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(StreamingObjectPath, [WeakThis, ExtraSection]() {
		if (UEstMenuWidget* StrongThis = WeakThis.Get())
		{
			if (!ExtraSection.IsValid())
			{
				return;
			}

			StrongThis->Extra(CreateWidget<UUserWidget>(StrongThis, ExtraSection.Get()));
		}
	},
		FStreamableManager::AsyncLoadHighPriority);
}

void UEstMenuWidget::Extra(UUserWidget* ExtraSection)
{
	if (MenuExtraContainer == nullptr)
	{
		return;
	}

	CurrentExtraSection = ExtraSection;

	MenuExtraContainer->AddChild(CurrentExtraSection);
}

void UEstMenuWidget::RemoveExtra()
{
	if (CurrentExtraSection != nullptr)
	{
		MenuExtraContainer->RemoveChild(CurrentExtraSection);
		CurrentExtraSection = nullptr;
	}
}

FReply UEstMenuWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	UWidget* WidgetToFocus = CurrentMenuModal != nullptr ? Cast<UWidget>(CurrentMenuModal) : CurrentMenuSection;
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

FReply UEstMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	AEstPlayerController* PC = Cast<AEstPlayerController>(GetOwningPlayer());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (PC && Subsystem && Subsystem->QueryKeysMappedToAction(PC->MenuAction).Contains(InKeyEvent.GetKey()))
	{
		ResumeGame();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UEstMenuWidget::ResumeGame()
{
	// Special handling for modal - ask it to exit
	if (CurrentMenuModal != nullptr)
	{
		CurrentMenuModal->OnBack();
		return;
	}

	UEstGameInstance* EstGameInstance = Cast<UEstGameInstance>(GetWorld()->GetGameInstance());
	AEstPlayerController* EstPlayerController = Cast<AEstPlayerController>(GetOwningPlayer());

	if (EstGameInstance != nullptr && EstPlayerController != nullptr && !EstGameInstance->GetMenuVisibleForever())
	{
		EstGameInstance->SetMenuVisibility(FEstMenuVisibilityContext());
		EstPlayerController->SetMenuFocusState(false);
		EstPlayerController->SetPause(false);
	}
}
