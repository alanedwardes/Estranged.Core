// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstHUDWidget.h"
#include "Runtime/Engine/Public/SubtitleManager.h"
#include "Gameplay/EstPlayer.h"
#include "Gameplay/EstPlayerController.h"
#include "Gameplay/EstFirearmWeapon.h"
#include "Gameplay/EstFirearmAmunition.h"
#include "EstCore.h"

extern ENGINE_API float GAverageFPS;
extern ENGINE_API float GAverageMS;

#define STATS_TEXT "%i (short) / %i (long)"
#define STATS_TEXT_CHEATS "%i (short) / %i (long) / cheats"

void UEstHUDWidget::NativeConstruct()
{
	Player = Cast<AEstPlayer>(GetOwningPlayerPawn());
	if (Player.IsValid())
	{
		Player->OnChangeWeapon.AddDynamic(this, &UEstHUDWidget::HandleChangeWeapon);
		Player->OnShowHint.AddDynamic(this, &UEstHUDWidget::HandleShowHint);
		Player->OnHideHint.AddDynamic(this, &UEstHUDWidget::HandleHideHint);
		Controller = Cast<AEstPlayerController>(Player->GetController());
		Firearm = Cast<AEstFirearmWeapon>(Player->EquippedWeapon.Get());
	}

	SubtitleFontSizeMultiplier = 1.0f;

	FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().AddUObject(this, &UEstHUDWidget::HandleSetSubtitleText);

	Super::NativeConstruct();
}

void UEstHUDWidget::NativeDestruct()
{
	FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().RemoveAll(this);

	if (Player.IsValid())
	{
		Player->OnTakePointDamage.RemoveAll(this);
		Player->OnChangeWeapon.RemoveAll(this);
		Player->OnShowHint.RemoveAll(this);
		Player->OnHideHint.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UEstHUDWidget::NewSubtitle(const FText &SubtitleText)
{
	if (Subtitles.Num() > 0)
	{
		FEstSubtitle *LastSubtitle = &Subtitles.Top();
		if (!LastSubtitle->bIsHidden)
		{
			LastSubtitle->bIsHidden = true;
			OnHideSubtitle(*LastSubtitle);
		}
	}

	if (!SubtitleText.IsEmpty())
	{
		FString SubtitleString = SubtitleText.ToString();

		FEstSubtitle NewSubtitle;
		NewSubtitle.bIsClosedCaption = SubtitleString.StartsWith(TEXT("[")) && SubtitleString.EndsWith(TEXT("]"));
		NewSubtitle.bIsHidden = false;
		NewSubtitle.SubtitleId = FGuid::NewGuid();
		NewSubtitle.SubtitleText = SubtitleText;
		NewSubtitle.bUseSimpleSubtitleFont = bUseSimpleSubtitleFont;
		NewSubtitle.SubtitleFontSizeMultiplier = SubtitleFontSizeMultiplier;
		Subtitles.Add(NewSubtitle);
		OnShowSubtitle(NewSubtitle);
	}

	if (Subtitles.Num() > 5)
	{
		OnDestroySubtitle(Subtitles[0]);
		Subtitles.RemoveAt(0);
	}
}

void UEstHUDWidget::HandleSetSubtitleText(const FText &SubtitleText)
{
	if (!LastSubtitleText.EqualTo(SubtitleText))
	{
		NewSubtitle(SubtitleText);
		LastSubtitleText = SubtitleText;
	}
}

void UEstHUDWidget::HandleShowHint(TArray<FName> Bindings, FText Label, float ShowTime, FVector WorldLocation)
{
	HintBindings = Bindings;
	HintLabel = Label;
	HintWorldLocation = WorldLocation;
	HintFinishTime = GetWorld()->TimeSeconds + (ShowTime > 0.f ? ShowTime : HintDuration);
}

void UEstHUDWidget::HandleHideHint()
{
	HintFinishTime = GetWorld()->TimeSeconds;
}

const TSet<FKey> UEstHUDWidget::GetHintKeys() const
{
	TSet<FKey> Keys;
	for (const FName &Binding : HintBindings)
	{
		const FKey AxisKey = UEstGameplayStatics::FindBestKeyForAxis(Controller.Get(), Binding, Controller->bIsUsingGamepad);
		const FKey ActionKey = UEstGameplayStatics::FindBestKeyForAction(Controller.Get(), Binding, Controller->bIsUsingGamepad);

		if (AxisKey != EKeys::Invalid)
		{
			Keys.Add(AxisKey);
		}

		if (ActionKey != EKeys::Invalid)
		{
			Keys.Add(ActionKey);
		}
	}

	return Keys;
}

const FString UEstHUDWidget::GetStatsForNerds() const
{
	return FString::Printf(TEXT("%5.2f FPS / %5.2f ms"), GAverageFPS, GAverageMS);
}

void UEstHUDWidget::HandleChangeWeapon(AEstBaseWeapon *Weapon)
{
	Firearm = Cast<AEstFirearmWeapon>(Weapon);
}

const FString UEstHUDWidget::GetClipLabel() const
{
	if (Firearm == nullptr)
	{
		return FString();
	}

	return FString::FromInt(Firearm->PrimaryAmmunition->ClipRemaining);
}

const FString UEstHUDWidget::GetAmmoLabel() const
{
	if (Firearm == nullptr)
	{
		return FString();
	}

	return FString::FromInt(Firearm->PrimaryAmmunition->AmmoRemaining);
}
