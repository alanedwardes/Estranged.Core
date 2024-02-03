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

void UEstHUDWidget::HandleShowHint(class UInputMappingContext* InputMappingContext, TArray<class UInputAction*> Bindings, FText Label, float ShowTime, FVector WorldLocation)
{
	HintInputMappingContext = InputMappingContext;
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
	if (HintInputMappingContext == nullptr)
	{
		return Keys;
	}

	TArray<FEnhancedActionKeyMapping> Mappings = HintInputMappingContext->GetMappings();
	const bool bIsUsingGamepad = Controller->bIsUsingGamepad;

	for (const UInputAction* Binding : HintBindings)
	{
		// Find the mapping which pertains to this input action
		FEnhancedActionKeyMapping* Found = Mappings.FindByPredicate([Binding, bIsUsingGamepad](FEnhancedActionKeyMapping Mapping)
		{
			return Mapping.Action == Binding && Mapping.Key.IsGamepadKey() == bIsUsingGamepad;
		});

		if (Found != nullptr && Found->Key != EKeys::Invalid)
		{
			Keys.Add(Found->Key);
		}
	}

	return Keys;
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
