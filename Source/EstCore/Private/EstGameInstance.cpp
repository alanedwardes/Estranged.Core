// Estranged is a trade mark of Alan Edwardes.

#include "EstGameInstance.h"
#include "Runtime/SlateCore/Public/Application/SlateApplicationBase.h"
#include "EstCore.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "EstAudioComponent.h"
#include "EstMenuWidget.h"
#include "EstGameplayStatics.h"
#include "Runtime/Engine/Public/AudioDevice.h"

void UEstGameInstance::Init()
{
	if (!IsDedicatedServerInstance())
	{
		MenuUserWidget = CreateWidget<UEstMenuWidget>(this, MenuWidgetType);
		MenuSlateWidget = MenuUserWidget->TakeWidget();
	}

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEstGameInstance::PreLoadMap);

	FTickerDelegate TickDelegate = FTickerDelegate::CreateUObject(this, &UEstGameInstance::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);

	Super::Init();
}

void UEstGameInstance::PreLoadMap(const FString & InMapName)
{
	FadeMusic();
	SetMenuVisibility(FEstMenuVisibilityContext(false, false));
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
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);

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