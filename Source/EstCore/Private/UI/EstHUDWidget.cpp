// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstHUDWidget.h"
#include "Runtime/Engine/Public/SubtitleManager.h"
#include "Gameplay/EstPlayer.h"
#include "Gameplay/EstPlayerController.h"
#include "Gameplay/EstFirearmWeapon.h"
#include "Gameplay/EstFirearmAmunition.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EstCore.h"

void UEstHUDWidget::NativeConstruct()
{
	Controller = GetOwningPlayer<AEstPlayerController>();

	Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &UEstHUDWidget::OnPossessedPawnChanged);

	OnPossessedPawnChanged(nullptr, Controller->GetPawn());

	FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().AddUObject(this, &UEstHUDWidget::HandleSetSubtitleText);

	Super::NativeConstruct();
}

void UEstHUDWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	OnChangePlayer(Cast<AEstPlayer>(OldPawn), Cast<AEstPlayer>(NewPawn));
}

void UEstHUDWidget::NativeDestruct()
{
	FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().RemoveAll(this);

	if (Player.IsValid())
	{
		OnPossessedPawnChanged(Player.Get(), nullptr);
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

void UEstHUDWidget::ShowHint(TArray<class UInputAction*> Bindings, FText Label, float ShowTime, FVector WorldLocation)
{
	HintBindings = Bindings;
	HintLabel = Label;
	HintWorldLocation = WorldLocation;
	HintFinishTime = GetWorld()->TimeSeconds + (ShowTime > 0.f ? ShowTime : HintDuration);
}

void UEstHUDWidget::HideHint()
{
	HintFinishTime = GetWorld()->TimeSeconds;
}

const TSet<FKey> UEstHUDWidget::GetHintKeys() const
{
	if (Controller.IsValid())
	{
		return UEstGameplayStatics::GetHintKeys(Controller->FirstPersonMappingContext, HintBindings, Controller.Get());
	}

	return TSet<FKey>();
}

void UEstHUDWidget::HandleChangeWeapon(AEstBaseWeapon *Weapon)
{
	Firearm = Cast<AEstFirearmWeapon>(Weapon);
}

void UEstHUDWidget::OnChangePlayer_Implementation(AEstPlayer* OldPlayer, AEstPlayer* NewPlayer)
{
	if (OldPlayer != nullptr)
	{
		OldPlayer->OnTakePointDamage.RemoveAll(this);
		OldPlayer->OnChangeWeapon.RemoveAll(this);
		OldPlayer->OnShowHint.RemoveAll(this);
		OldPlayer->OnHideHint.RemoveAll(this);
	}

	if (NewPlayer != nullptr)
	{
		Player = NewPlayer;
		NewPlayer->OnChangeWeapon.AddUniqueDynamic(this, &UEstHUDWidget::HandleChangeWeapon);
		NewPlayer->OnShowHint.AddUniqueDynamic(this, &UEstHUDWidget::ShowHint);
		NewPlayer->OnHideHint.AddUniqueDynamic(this, &UEstHUDWidget::HideHint);
		Firearm = Cast<AEstFirearmWeapon>(NewPlayer->EquippedWeapon.Get());
	}
	else
	{
		Player = nullptr;
		Firearm = nullptr;
	}
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
