// Estranged is a trade mark of Alan Edwardes.

#include "EstHUDWidget.h"
#include "Runtime/Engine/Public/SubtitleManager.h"
#include "EstPlayer.h"
#include "EstCore.h"

extern ENGINE_API float GAverageFPS;

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
		FEstSubtitle NewSubtitle;
		NewSubtitle.bIsHidden = false;
		NewSubtitle.bIsCaption = Captions.FindByPredicate([&](const FText Item) { return Item.EqualTo(SubtitleText); }) != nullptr;
		NewSubtitle.SubtitleId = FGuid::NewGuid();
		NewSubtitle.SubtitleText = SubtitleText;
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

void UEstHUDWidget::HandleShowHint(TArray<FName> Bindings, FText Label, bool bShowUntilHidden, FVector WorldLocation)
{
	HintBindings = Bindings;
	HintLabel = Label;
	HintWorldLocation = WorldLocation;
	HintFinishTime = GetWorld()->TimeSeconds + (bShowUntilHidden ? BIG_NUMBER : HintDuration);
}

void UEstHUDWidget::HandleHideHint()
{
	HintFinishTime = GetWorld()->TimeSeconds;
}

const FString UEstHUDWidget::GetHintKeyLabels() const
{
	TSet<FString> KeyLabelsTemp;
	for (const FName Binding : HintBindings)
	{
		const FKey AxisKey = UEstGameplayStatics::FindBestKeyForAxis(Controller.Get(), Binding, Controller->bIsUsingGamepad);
		const FKey ActionKey = UEstGameplayStatics::FindBestKeyForAction(Controller.Get(), Binding, Controller->bIsUsingGamepad);

		if (AxisKey != EKeys::Invalid)
		{
			KeyLabelsTemp.Add(UEstGameplayStatics::GetKeyDisplayName(AxisKey));
		}

		if (ActionKey != EKeys::Invalid)
		{
			KeyLabelsTemp.Add(UEstGameplayStatics::GetKeyDisplayName(ActionKey));
		}
	}

	FString KeyLabelTemp = FString::Join(KeyLabelsTemp.Array(), TEXT(" + "));
	KeyLabelTemp.ToUpperInline();
	KeyLabelTemp.ReplaceInline(TEXT("GAMEPAD"), TEXT(""));
	KeyLabelTemp.ReplaceInline(TEXT("BUTTON"), TEXT(""));
	KeyLabelTemp.TrimStartAndEndInline();

	return KeyLabelTemp;
}

const TSet<FKey> UEstHUDWidget::GetHintKeys() const
{
	TSet<FKey> Keys;
	for (const FName Binding : HintBindings)
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
	return FString::Printf(TEXT(STATS_TEXT), FMath::RoundToInt(GAverageFPS), FMath::RoundToInt(FEstCoreModule::GetLongAverageFrameRate()));
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
