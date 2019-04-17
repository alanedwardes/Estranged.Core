// Estranged is a trade mark of Alan Edwardes.


#include "EstAudioComponent.h"

UEstAudioComponent::UEstAudioComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEstAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsPlaying() && Sound != nullptr)
	{
		const float PlayPosition = GetPlayPosition();
		const float TimeLeft = Sound->Duration - PlayPosition;

		const float FadeInMultiplier = FadeInTime > 0.f ? FMath::Clamp(PlayPosition / FadeInTime, .01f, 1.f) : 1.f;
		const float FadeOutMultiplier = FadeOutTime > 0.f ? FMath::Clamp(TimeLeft / FadeOutTime, 0.f, 1.f) : 1.f;
		SetVolumeMultiplier(FadeInMultiplier * FadeOutMultiplier);

		// Leave a bufffer otherwise there may be a noticable gap for loops
		if (PlayPosition - .1f >= Sound->Duration)
		{
			Stop();
			OnSoundPlayed.Broadcast();
		}
	}
}

void UEstAudioComponent::OnPostRestore_Implementation()
{
	SetSound(SAVE_Sound);

	if (SAVE_bIsPlaying)
	{
		Play(SAVE_Position);
	}
}

void UEstAudioComponent::OnPreSave_Implementation()
{
	SAVE_Position = GetPlayPosition();
	SAVE_bIsPlaying = IsPlaying();
	SAVE_Sound = Sound;
}

void UEstAudioComponent::Play(float StartTime)
{
	Super::Play(StartTime);
	SoundGameStartTime = GetWorld()->TimeSeconds - StartTime;
}

float UEstAudioComponent::GetPlayPosition()
{
	if (!IsPlaying())
	{
		return 0.f;
	}

	return GetWorld()->TimeSeconds - SoundGameStartTime;
}
