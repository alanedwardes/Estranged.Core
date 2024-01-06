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
#include "Gameplay/EstSaveStatics.h"
#include "Saves/EstGameSettingsSave.h"

void UEstGameInstance::Init()
{
	if (!IsDedicatedServerInstance())
	{
		MenuUserWidget = CreateWidget<UEstMenuWidget>(this, MenuWidgetType);
		MenuSlateWidget = MenuUserWidget->TakeWidget();
	}

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEstGameInstance::PreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UEstGameInstance::PostLoadMapWithWorld);

	FTickerDelegate TickDelegate = FTickerDelegate::CreateUObject(this, &UEstGameInstance::Tick);
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);

	Super::Init();
}

void UEstGameInstance::PreLoadMap(const FString & InMapName)
{
	FadeMusic();
	SetMenuVisibility(FEstMenuVisibilityContext(false, false));
}

void UEstGameInstance::RefreshLoggerState()
{
	if (bLoggerEnabled)
	{
		if (!LoggerUserWidget.IsValid())
		{
			LoggerUserWidget = CreateWidget<UEstLoggerWidget>(this, LoggerWidgetType);
		}

		if (!LoggerUserWidget->IsInViewport())
		{
			LoggerUserWidget->AddToViewport();
		}
	}
	else
	{
		if (LoggerUserWidget.IsValid() && LoggerUserWidget->IsInViewport())
		{
			LoggerUserWidget->RemoveFromParent();
			LoggerUserWidget.Reset();
		}
	}
}

void UEstGameInstance::PostLoadMapWithWorld(UWorld* World)
{
	RefreshLoggerState();
	UGameplayStatics::GetPlayerCameraManager(World, 0)->StartCameraFade(1.f, 0.f, 1.f, FLinearColor::Black, false, false);
}

void UEstGameInstance::SetLoggerEnabled(bool bNewLoggerEnabled)
{
	bLoggerEnabled = bNewLoggerEnabled;
	RefreshLoggerState();
}

bool UEstGameInstance::GetLoggerEnabled()
{
	return bLoggerEnabled;
}

void UEstGameInstance::SetCheatsEnabled(bool bNewCheatsEnabled)
{
	bCheatsEnabled = bNewCheatsEnabled;
}

bool UEstGameInstance::GetCheatsEnabled()
{
	return bCheatsEnabled;
}

void UEstGameInstance::OnStart()
{
#if WITH_EDITOR
	SetCheatsEnabled(true);
	SetLoggerEnabled(true);
#endif
	ApplyAudioSettings(UEstSaveStatics::LoadGameSettings());
}

void UEstGameInstance::LogMessage(FEstLoggerMessage Message)
{
	if (LoggerUserWidget.IsValid() && bLoggerEnabled)
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

void UEstGameInstance::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);

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
		MenuUserWidget->SetOwningPlayer(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		MenuUserWidget->AddToViewport();
		MenuUserWidget->OnShowMenu(InVisibilityContext.MenuSection);

		// Forcefully enable the mouse cursor at the platform level. This works around a bug in the Ansel plugin
		UEstGameplayStatics::SetPlatformMouseCursorState(true);
	}
	else if (!VisibilityContext.bIsMenuVisibleForever)
	{
		MenuUserWidget->OnHideMenu();
		MenuUserWidget->RemoveFromParent();
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

	FAudioDeviceHandle AudioDeviceHandle = GEngine->GetActiveAudioDevice();
	if (!AudioDeviceHandle.IsValid())
	{
		return false;
	}

	AudioComponent = NewObject<UAudioComponent>();
	AudioComponent->AddToRoot();
	AudioComponent->AudioDeviceID = AudioDeviceHandle.GetDeviceID();
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

void UEstGameInstance::ApplyAudioSettings(UEstGameSettingsSave* GameSettings)
{
	FAudioDeviceHandle AudioDeviceHandle = GEngine->GetActiveAudioDevice();
	if (!AudioDeviceHandle.IsValid())
	{
		return;
	}

	const float Pitch = 1.f;
	const float FadeInTime = 0.f;
	AudioDeviceHandle.GetAudioDevice()->SetSoundMixClassOverride(SoundMix, EffectsSoundClass, GameSettings->EffectsVolume * GameSettings->MasterVolume, Pitch, FadeInTime, true);
	AudioDeviceHandle.GetAudioDevice()->SetSoundMixClassOverride(SoundMix, VoiceSoundClass, GameSettings->VoiceVolume * GameSettings->MasterVolume, Pitch, FadeInTime, true);
	AudioDeviceHandle.GetAudioDevice()->SetSoundMixClassOverride(SoundMix, MusicSoundClass, GameSettings->MusicVolume * GameSettings->MasterVolume, Pitch, FadeInTime, true);
}