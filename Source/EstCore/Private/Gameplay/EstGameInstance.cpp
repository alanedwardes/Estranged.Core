// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstGameInstance.h"
#include "Runtime/SlateCore/Public/Application/SlateApplicationBase.h"
#include "EstCore.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Sound/EstAudioComponent.h"
#include "UI/EstMenuWidget.h"
#include "UI/EstLoggerWidget.h"
#include "Gameplay/EstGameplayStatics.h"
#include "Runtime/Engine/Public/AudioDevice.h"
#include "Online.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"

UEstGameInstance::UEstGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetPrimaryPlayer();
}

void UEstGameInstance::Init()
{
	if (!IsDedicatedServerInstance())
	{
		MenuUserWidget = CreateWidget<UEstMenuWidget>(this, MenuWidgetType);
		MenuSlateWidget = MenuUserWidget->TakeWidget();

		LoggerUserWidget = CreateWidget<UEstLoggerWidget>(this, LoggerWidgetType);
		LoggerSlateWidget = LoggerUserWidget->TakeWidget();
	}

	FSlateApplication::Get().OnUserRegistered().AddUObject(this, &UEstGameInstance::OnUserRegistered);

	FCoreDelegates::OnControllerPairingChange.AddUObject(this, &UEstGameInstance::OnControllerPairingChange);
	FCoreDelegates::OnControllerConnectionChange.AddUObject(this, &UEstGameInstance::OnControllerConnectionChange);
	FCoreDelegates::OnUserLoginChangedEvent.AddUObject(this, &UEstGameInstance::OnUserLoginChangedEvent);

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEstGameInstance::PreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UEstGameInstance::PostLoadMapWithWorld);

	FTickerDelegate TickDelegate = FTickerDelegate::CreateUObject(this, &UEstGameInstance::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);

	Super::Init();
}

void UEstGameInstance::Shutdown()
{
	FSlateApplication::Get().OnUserRegistered().RemoveAll(this);

	FCoreDelegates::OnControllerPairingChange.RemoveAll(this);
	FCoreDelegates::OnControllerConnectionChange.RemoveAll(this);
	FCoreDelegates::OnUserLoginChangedEvent.RemoveAll(this);

	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);

	MenuSlateWidget = nullptr;
	LoggerSlateWidget = nullptr;

	Super::Shutdown();
}

bool UEstGameInstance::IsPrimaryPlayer(FPlatformUserId UserPlatformId, int32 ControllerIndex)
{
	return CurrentUserPlatformId == UserPlatformId && CurrentControllerIndex == ControllerIndex;
}

void UEstGameInstance::SetPrimaryPlayer(FPlatformUserId UserPlatformId, int32 ControllerIndex)
{
	CurrentUserPlatformId = UserPlatformId;
	CurrentControllerIndex = ControllerIndex;
}

bool UEstGameInstance::IsUserLoggedIn()
{
	return !IsPrimaryPlayer(PLATFORMUSERID_NONE, INDEX_NONE);
}

void UEstGameInstance::PrintPrimaryPlayer()
{
	UKismetSystemLibrary::PrintText(this, FText::Format(FText::FromString("UEstGameInstance::PrintPrimaryPlayer UserPlatformId={0} ControllerIndex={1}"), CurrentUserPlatformId, CurrentControllerIndex));
}

void UEstGameInstance::OnUserLoginChangedEvent(bool IsLogin, int32 UserId, int32 UserIndex)
{
	PrintPrimaryPlayer();
	UKismetSystemLibrary::PrintText(this, FText::Format(FText::FromString("FCoreDelegates::OnUserLoginChangedEvent IsLogin={0} UserId={1} UserIndex={2}"), FText::FromString(IsLogin ? TEXT("True") : TEXT("False")), UserId, UserIndex));
}

void UEstGameInstance::OnControllerConnectionChange(bool IsConnection, FPlatformUserId UserPlatformId, int32 ControllerIndex)
{
	PrintPrimaryPlayer();
	UKismetSystemLibrary::PrintText(this, FText::Format(FText::FromString("FCoreDelegates::OnControllerConnectionChange IsConnection={0} UserPlatformId={1} ControllerIndex={2}"), FText::FromString(IsConnection ? TEXT("True") : TEXT("False")), UserPlatformId, ControllerIndex));

	if (IsPrimaryPlayer(UserPlatformId, ControllerIndex))
	{
		OnCurrentUserConnectionChange(IsConnection);
	}
}

void UEstGameInstance::OnControllerPairingChange(int32 ControllerIndex, FPlatformUserId NewUserPlatformId, FPlatformUserId OldUserPlatformId)
{
	PrintPrimaryPlayer();
	UKismetSystemLibrary::PrintText(this, FText::Format(FText::FromString("FCoreDelegates::OnControllerPairingChanged ControllerIndex={0} NewUserPlatformId={1} OldUserPlatformId={2}"), ControllerIndex, NewUserPlatformId, OldUserPlatformId));

	// Set the indexes if none are set yet
	if (!IsUserLoggedIn())
	{
		SetPrimaryPlayer(NewUserPlatformId, ControllerIndex);
		return;
	}

	if (IsPrimaryPlayer(OldUserPlatformId, ControllerIndex))
	{
		IOnlineIdentityPtr IdentityInterface = Online::GetIdentityInterface();
		auto LoginStatus = IdentityInterface->GetLoginStatus(ControllerIndex);

		UKismetSystemLibrary::PrintText(this, FText::Format(FText::FromString("FCoreDelegates::OnControllerPairingChanged Status={0}"), LoginStatus));

		// The primary player signed out, force to the menu screen
		if (NewUserPlatformId == PLATFORMUSERID_NONE)
		{
			OnCurrentUserLoggedOut();
			return;
		}
	}
}

void UEstGameInstance::OnUserRegistered(int32 UserIndex)
{
	PrintPrimaryPlayer();
	UKismetSystemLibrary::PrintText(this, FText::FromString("FSlateApplication::OnUserRegistered"));

	// Force the new user to focus on whatever existing users are focused on
	TSharedPtr<SWidget> FirstFocusedWidget;
	FSlateApplication::Get().ForEachUser([&FirstFocusedWidget](FSlateUser& User)
	{
		TSharedPtr<SWidget> FocusedWidget = User.GetFocusedWidget();
		if (FocusedWidget.IsValid())
		{
			FirstFocusedWidget = FocusedWidget;
		}
	});

	if (FirstFocusedWidget.IsValid())
	{
		TSharedPtr<FSlateUser> User = FSlateApplication::Get().GetUser(UserIndex);
		if (!User->GetFocusedWidget().IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("******************************************* Setting user %i focus"), User->GetUserIndex());
			FSlateApplication::Get().SetUserFocus(UserIndex, FirstFocusedWidget.ToSharedRef());
		}
	}
}

void UEstGameInstance::PreLoadMap(const FString & InMapName)
{
	FadeMusic();
	SetLoggerVisible(false);
	SetMenuVisibility(FEstMenuVisibilityContext(false, false));
}

void UEstGameInstance::RefreshLoggerState()
{
	SetLoggerVisible(bIsLoggerEnabled);
}

void UEstGameInstance::PostLoadMapWithWorld(UWorld* World)
{
	RefreshLoggerState();
}

void UEstGameInstance::SetLoggerEnabled(bool NewIsEnabled)
{
	if (NewIsEnabled)
	{
		bIsLoggerEnabled = true;
		RefreshLoggerState();
	}
	else
	{
		bIsLoggerEnabled = false;
		RefreshLoggerState();
	}
}

void UEstGameInstance::SetCurrentUserFromUserIndex(int32 LocalUserIndex)
{
	IOnlineIdentityPtr IdentityInterface = Online::GetIdentityInterface();
	TSharedPtr<const FUniqueNetId> UniquePlayerId = IdentityInterface->GetUniquePlayerId(LocalUserIndex);
	FPlatformUserId PlatformUserId = IdentityInterface->GetPlatformUserIdFromUniqueNetId(*UniquePlayerId);

	UKismetSystemLibrary::PrintText(this, FText::Format(FText::FromString("UEstGameInstance::SetCurrentUserFromUserIndex LocalUserIndex={0} UniquePlayerId={1} PlatformUserId={2}"), LocalUserIndex, FText::FromString(UniquePlayerId->ToDebugString()), PlatformUserId));
	SetPrimaryPlayer(PlatformUserId, LocalUserIndex);
}

void UEstGameInstance::SetLoggerVisible(bool NewIsVisible)
{
	if (NewIsVisible == bIsLoggerVisible)
	{
		return;
	}

	bIsLoggerVisible = NewIsVisible;
	if (bIsLoggerVisible)
	{
		LoggerUserWidget->AddToViewport();
	}
	else
	{
		LoggerUserWidget->RemoveFromViewport();
	}
}

void UEstGameInstance::OnStart()
{
	SetLoggerVisible(true);
}

void UEstGameInstance::LogMessage(FEstLoggerMessage Message)
{
	if (LoggerUserWidget != nullptr && bIsLoggerEnabled)
	{
		LoggerUserWidget->OnLogMessage(Message);
	}
}

void UEstGameInstance::FadeMusic()
{
	if (AudioComponent == nullptr)
	{
		return;
	}

	const float FadeOutTime = 5.f;
	AudioComponent->FadeOut(FadeOutTime, 0.f);
	MusicFadeCompleteTime = GameInstanceTime + FadeOutTime;
	NextMusic.Reset();
	bWasFadingOut = true;
}

void UEstGameInstance::StopMusic()
{
	if (AudioComponent != nullptr)
	{
		AudioComponent->Stop();
		NextMusic.Reset();
	}
}

void UEstGameInstance::PlayMusic(FEstMusic Music)
{
	if (LazilyCreateAudioComponent())
	{
		if (!AudioComponent->IsPlaying())
		{
			PlayMusicInternal(Music);
		}
		else if (Music.bShouldFadeCurrent)
		{
			FadeMusic();
			NextMusic = Music;
		}
		else
		{
			NextMusic = Music;
		}
	}
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
		MenuUserWidget->SetOwningPlayer(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		MenuUserWidget->AddToViewport();
		MenuUserWidget->ShowMenu(InVisibilityContext.MenuSection);

		// Forcefully enable the mouse cursor at the platform level. This works around a bug in the Ansel plugin
		UEstGameplayStatics::SetPlatformMouseCursorState(true);
	}
	else if (!VisibilityContext.bIsMenuVisibleForever)
	{
		MenuUserWidget->RemoveFromViewport();
	}
}

FEstMusic UEstGameInstance::GetMusic()
{
	// Shortcut the next transition
	if (NextMusic.IsSet())
	{
		return NextMusic.GetValue();
	}

	if (AudioComponent == nullptr || !AudioComponent->IsPlaying() || bWasFadingOut)
	{
		return FEstMusic();
	}

	FEstMusic Music;
	Music.Sound = AudioComponent->Sound;
	Music.Position = GetPlayPosition();
	Music.bShouldFadeCurrent = true;
	return Music;
}

bool UEstGameInstance::LazilyCreateAudioComponent()
{
	if (AudioComponent != nullptr)
	{
		return true;
	}

	FAudioDevice* AudioDevice = GEngine->GetActiveAudioDevice();
	if (AudioDevice == nullptr)
	{
		return false;
	}

	AudioComponent = NewObject<UAudioComponent>();
	AudioComponent->AddToRoot();
	AudioComponent->AudioDeviceHandle = AudioDevice->DeviceHandle;
	AudioComponent->SetVolumeMultiplier(1.0f);
	AudioComponent->SetPitchMultiplier(1.0f);
	AudioComponent->bAllowSpatialization = false;
	AudioComponent->bIsUISound = true;
	AudioComponent->bAutoDestroy = false;
	AudioComponent->bIgnoreForFlushing = true;
	AudioComponent->bIsMusic = true;
	AudioComponent->SubtitlePriority = -1.f;
	AudioComponent->SoundClassOverride = MusicSoundClass;

	return true;
}

float UEstGameInstance::GetPlayPosition()
{
	if (AudioComponent == nullptr)
	{
		return 0.f;
	}

	return UEstGameplayStatics::GetPlayPositionWithinLoop(AudioComponent, GameInstanceTime - MusicStartTime);
}

void UEstGameInstance::PlayMusicInternal(FEstMusic Music)
{
	if (LazilyCreateAudioComponent())
	{
		AudioComponent->SetSound(Music.Sound);

		if (Music.bNoFadeIn)
		{
			AudioComponent->Play(Music.Position);
		}
		else
		{
			AudioComponent->FadeIn(5.f, 1.f, Music.Position);
		}

		bWasFadingOut = false;
		MusicStartTime = GameInstanceTime;
	}
}

bool UEstGameInstance::Tick(float DeltaTime)
{
	GameInstanceTime += DeltaTime;

	if (AudioComponent != nullptr)
	{
		if (NextMusic.IsSet())
		{
			bool bIsLooping = UEstGameplayStatics::IsLooping(AudioComponent);
			bool bIsSuitableStopPoint = UEstGameplayStatics::IsSuitableStopPoint(AudioComponent, GetPlayPosition());
			bool bIsPlaying = AudioComponent->IsPlaying();
			bool bIsFadeCompleted = !bWasFadingOut || MusicFadeCompleteTime < GameInstanceTime;

			if (!bIsPlaying || (bIsLooping && bIsSuitableStopPoint && bIsFadeCompleted))
			{
				FEstMusic Next = NextMusic.GetValue();
				NextMusic.Reset();
				PlayMusicInternal(Next);
			}
		}
	}

	return true;
}