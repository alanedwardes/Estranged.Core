// Estranged is a trade mark of Alan Edwardes.

#include "EstHUDWidget.h"
#include "Runtime/Engine/Public/SubtitleManager.h"
#include "EstCore.h"

void UEstHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	LastSubtitleTime = -BIG_NUMBER;

	//Player = Cast<AEstPlayer>(GetOwningPlayerPawn());
	//if (Player.IsValid())
	//{
	//	//Player->OnTakeAnyDamage.AddDynamic(this, &AEstPlayerHUD::HandleDamage);
	//	//Player->OnChangeWeapon.AddDynamic(this, &AEstPlayerHUD::HandleChangeWeapon);
	//	//Player->OnShowHint.AddDynamic(this, &AEstPlayerHUD::HandleShowHint);
	//	//Player->OnHideHint.AddDynamic(this, &AEstPlayerHUD::HandleHideHint);
	//	//Controller = Cast<AEstPlayerController>(Player->GetController());
	//	//Firearm = Cast<AEstFirearmWeapon>(Player->EquippedWeapon.Get());
	//}

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
