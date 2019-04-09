// Estranged is a trade mark of Alan Edwardes.

#include "EstHUDWidget.h"
#include "Runtime/Engine/Public/SubtitleManager.h"
#include "EstCore.h"

void UEstHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	LastSubtitleTime = -BIG_NUMBER;

	Player = Cast<AEstPlayer>(GetOwningPlayerPawn());
	if (Player.IsValid())
	{
		//Player->OnTakeAnyDamage.AddDynamic(this, &AEstPlayerHUD::HandleDamage);
		//Player->OnChangeWeapon.AddDynamic(this, &AEstPlayerHUD::HandleChangeWeapon);
		Player->OnShowHint.AddDynamic(this, &UEstHUDWidget::HandleShowHint);
		Player->OnHideHint.AddDynamic(this, &UEstHUDWidget::HandleHideHint);
		Controller = Cast<AEstPlayerController>(Player->GetController());
		//Firearm = Cast<AEstFirearmWeapon>(Player->EquippedWeapon.Get());
	}

	FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().AddUObject(this, &UEstHUDWidget::HandleSetSubtitleText);
}

void UEstHUDWidget::NativeDestruct()
{
	FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().RemoveAll(this);

	Super::NativeDestruct();
}

void UEstHUDWidget::HandleSetSubtitleText(const FText & SubtitleText)
{
	const bool bHasSubtitle = !SubtitleText.IsEmpty();
	const bool bDuringPaddingPeriod = GetWorld()->TimeSince(LastSubtitleTime) < 1.f;

	bShouldDrawSubtitles = bHasSubtitle || bDuringPaddingPeriod;

	if (bHasSubtitle)
	{
		LastSubtitleText = SubtitleText;
		LastSubtitleTime = GetWorld()->TimeSeconds;
	}

	const float TargetAlpha = bShouldDrawSubtitles ? 1.f : 0.f;
	SubtitleAlpha = FMath::FInterpTo(SubtitleAlpha, TargetAlpha, GetWorld()->DeltaTimeSeconds, 10.f);
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