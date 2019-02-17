#include "EstPlayerHUD.h"
#include "EstCore.h"
#include "EstPlayer.h"
#include "EstPlayerController.h"
#include "EstFirearmWeapon.h"
#include "EstFirearmAmunition.h"
#include "EstResourceComponent.h"
#include "EstHealthComponent.h"
#include "Runtime/Engine/Classes/Engine/Canvas.h"
#include "Runtime/Engine/Public/SubtitleManager.h"
#include "Kismet/KismetMathLibrary.h"

void AEstPlayerHUD::BeginPlay()
{
	LastSubtitleTime = -BIG_NUMBER;

	Super::BeginPlay();

	Player = Cast<AEstPlayer>(GetOwningPawn());
	if (Player.IsValid())
	{
		Player->OnTakeAnyDamage.AddDynamic(this, &AEstPlayerHUD::HandleDamage);
		Player->OnChangeWeapon.AddDynamic(this, &AEstPlayerHUD::HandleChangeWeapon);
		Player->OnShowHint.AddDynamic(this, &AEstPlayerHUD::HandleShowHint);
		Player->OnHideHint.AddDynamic(this, &AEstPlayerHUD::HandleHideHint);
		FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().AddUObject(this, &AEstPlayerHUD::HandleSetSubtitleText);
		Controller = Cast<AEstPlayerController>(Player->GetController());
		Firearm = Cast<AEstFirearmWeapon>(Player->EquippedWeapon.Get());
	}
}

void AEstPlayerHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (Player.IsValid())
	{
		Player->OnTakePointDamage.RemoveAll(this);
		Player->OnChangeWeapon.RemoveAll(this);
		Player->OnShowHint.RemoveAll(this);
		Player->OnHideHint.RemoveAll(this);
		FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().RemoveAll(this);
	}
}

void AEstPlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	if (bDisableHUD)
	{
		return;
	}

	if (!Player.IsValid())
	{
		return;
	}

	if (!Controller.IsValid())
	{
		return;
	}

	const float FadeTarget = ShouldDrawHUD() ? 1.f : 0.f;
	const float HudFadeTime = 4.f;
	HudColor.A = FMath::FInterpTo(HudColor.A, FadeTarget, GetWorld()->DeltaTimeSeconds, HudFadeTime);

	if (!Controller->bShowMouseCursor)
	{
		DrawReticule();
		DrawHealthIndicator();
		DrawBatteryIndicator();
		DrawDamageIndicators();
		DrawAmmoLabels();
		DrawHint();
		DrawLoadingIndicator();
		DrawSubtitles();
	}

	DrawGameSpecificHUD();

	LastCanvasSizeX = Canvas->SizeX;
	LastCanvasSizeY = Canvas->SizeY;
}

void AEstPlayerHUD::DrawGameSpecificHUD()
{
}

bool AEstPlayerHUD::ShouldDrawHUD()
{
	if (Controller->PlayerCameraManager->FadeAmount > 0.f)
	{
		return false;
	}

	if (!Player->IsViewTarget())
	{
		return false;
	}

	if (Player->ActorHasTag(TAG_DEAD))
	{
		return false;
	}

	return true;
}

void AEstPlayerHUD::DrawAmmoLabels()
{
	if (!Firearm.IsValid())
	{
		return;
	}

	const FString ClipLabel = FString::FromInt(Firearm->PrimaryAmmunition->ClipRemaining);
	const FString AmmoLabel = FString::FromInt(Firearm->PrimaryAmmunition->AmmoRemaining);

	float RoundsLabelRoundsWidth;
	float RoundsLabelRoundsHeight;
	GetTextSize(ClipLabel, RoundsLabelRoundsWidth, RoundsLabelRoundsHeight, AmmoLabelRoundsFont);
	RoundsLabelRoundsHeight *= .75f;

	float MagazinesLabelRoundsWidth;
	float MagazinesLabelRoundsHeight;
	GetTextSize(AmmoLabel, MagazinesLabelRoundsWidth, MagazinesLabelRoundsHeight, AmmoLabelMagazinesFont);

	const float PositionX = Canvas->SizeX * AmmoLabelPosition.X;
	const float PositionY = Canvas->SizeY * AmmoLabelPosition.Y;

	const FLinearColor TextColor = FLinearColor(1.f, 1.f, 1.f, HudColor.A);
	const FLinearColor ShadowColor = FLinearColor(0.f, 0.f, 0.f, HudColor.A);

	DrawText(ClipLabel, ShadowColor, PositionX + 1, PositionY + 1, AmmoLabelRoundsFont);
	DrawText(ClipLabel, TextColor, PositionX, PositionY, AmmoLabelRoundsFont);

	const float MagazinesPositionX = (PositionX + RoundsLabelRoundsWidth) - MagazinesLabelRoundsWidth;
	DrawText(AmmoLabel, ShadowColor, MagazinesPositionX + 1, PositionY + RoundsLabelRoundsHeight + 1, AmmoLabelMagazinesFont);
	DrawText(AmmoLabel, TextColor, MagazinesPositionX, PositionY + RoundsLabelRoundsHeight, AmmoLabelMagazinesFont);
}

void AEstPlayerHUD::DrawHint()
{
	if (HintLabel.IsEmpty())
	{
		return;
	}

	const float FadeTarget = GetWorld()->TimeSeconds < HintFinishTime ? HudColor.A : 0.f;
	HintLabelColor.A = FMath::FInterpTo(HintLabelColor.A, FadeTarget, GetWorld()->DeltaTimeSeconds, 5.f);

	if (FMath::IsNearlyZero(HintLabelColor.A))
	{
		return;
	}

	FVector HintProjectedLocation = Canvas->Project(HintWorldLocation);

	const bool bIsWorldProjected = !HintWorldLocation.IsZero();
	const bool bIsFacing = HintProjectedLocation.Z > 0.f;
	if (!bIsFacing && bIsWorldProjected)
	{
		return;
	}

	const FString KeyLabel = HintBindings.Num() > 0 ? GetHintKeyLabels() : FString("INFO");

	float KeyLabelWidth;
	float KeyLabelHeight;
	GetTextSize(KeyLabel, KeyLabelWidth, KeyLabelHeight, HintKeyFont);

	float HintLabelWidth;
	float HintLabelHeight;
	GetTextSize(HintLabel.ToString(), HintLabelWidth, HintLabelHeight, HintLabelFont);

	const FVector2D HintPadding = FVector2D(HintLabelPadding.X * Canvas->SizeX, HintLabelPadding.Y * Canvas->SizeY);
	const FVector2D HintSize = FVector2D(HintPadding.X + KeyLabelWidth + HintPadding.X + HintLabelWidth + HintPadding.X, HintPadding.Y + FMath::Max(KeyLabelHeight, HintLabelHeight) + HintPadding.Y);

	FVector2D HintPosition = FVector2D(HintProjectedLocation.X, HintProjectedLocation.Y);
	if (!bIsWorldProjected)
	{
		HintPosition = FVector2D((Canvas->SizeX * HintLabelPosition.X) - (HintSize.X * .5f), (Canvas->SizeY * HintLabelPosition.Y) - (HintSize.Y * .5f));
	}

	const FLinearColor TextColor = FLinearColor(1.f, 1.f, 1.f, HintLabelColor.A);

	DrawRect(HintLabelColor, HintPosition.X, HintPosition.Y, HintSize.X, HintSize.Y);
	DrawText(KeyLabel, TextColor, HintPosition.X + HintPadding.X, HintPosition.Y + HintPadding.Y, HintKeyFont);
	DrawText(HintLabel.ToString(), TextColor, HintPosition.X + HintPadding.X + KeyLabelWidth + HintPadding.X, HintPosition.Y + HintPadding.Y, HintLabelFont);
}

void AEstPlayerHUD::DrawLoadingIndicator()
{
	if (!bIsLoading)
	{
		return;
	}

	const FString LoadingLabel = LoadingLabelText.ToString();

	float LabelWidth;
	float LabelHeight;
	GetTextSize(LoadingLabel, LabelWidth, LabelHeight, LoadingLabelFont);

	const float BoxHeight = LabelHeight * 2.f;
	const float VerticalCenter = float(Canvas->SizeY) * .5f;
	const float HorizontalCenter = float(Canvas->SizeX) * .5f;

	DrawRect(FLinearColor::Black, 0.f, VerticalCenter - (BoxHeight * .5f), float(Canvas->SizeX), BoxHeight);
	DrawText(LoadingLabel, FLinearColor::White, HorizontalCenter - (LabelWidth * .5f), VerticalCenter - (LabelHeight * .5f), LoadingLabelFont);
}

void AEstPlayerHUD::DrawSubtitles()
{
	if (bDisableSubtitles)
	{
		return;
	}

	const float VerticalCenter = float(Canvas->SizeY) * .8f;
	const float HorizontalCenter = float(Canvas->SizeX) * .5f;

	UFont* SubtitleFont = Canvas->SizeX < 1750 ? SubtitleFontSmall : SubtitleFontLarge;

	const float TargetAlpha = bShouldDrawSubtitles ? 1.f : 0.f;
	SubtitleAlpha = FMath::FInterpTo(SubtitleAlpha, TargetAlpha, GetWorld()->DeltaTimeSeconds, 10.f);

	if (FMath::IsNearlyZero(SubtitleAlpha))
	{
		return;
	}

	const FString SubtitleText = LastSubtitleText.ToString();

	const FLinearColor TextColor = FLinearColor(1.f, 1.f, 1.f, SubtitleAlpha);
	const FLinearColor ShadowColor = FLinearColor(0.f, 0.f, 0.f, SubtitleAlpha);
	const FLinearColor BackgroundColor = FLinearColor(0.f, 0.f, 0.f, 0.5f * SubtitleAlpha);

	float SubtitleWidth;
	float SubtitleHeight;
	GetTextSize(SubtitleText, SubtitleWidth, SubtitleHeight, SubtitleFont);

	// Scale subtiles based on the width of the screen, the font's size and a multiplier
	const float SubtitleScale = float(Canvas->SizeX) / float(SubtitleFont->LegacyFontSize * 85);
	SubtitleWidth *= SubtitleScale;
	SubtitleHeight *= SubtitleScale;

	float SubtitlePositionX = FMath::RoundToInt(HorizontalCenter - (SubtitleWidth * .5f));
	float SubtitlePositionY = FMath::RoundToInt(VerticalCenter - (SubtitleHeight * .5f));

	const float SubtitlePaddingX = .01f * Canvas->SizeX;
	const float SubtitlePaddingY = .005f * Canvas->SizeX;

	const float TargetSubtitleBoxX = SubtitlePositionX - SubtitlePaddingX;
	const float TargetSubtitleBoxW = SubtitleWidth + (SubtitlePaddingX * 2.f);

	// Don't animate from zero or when resolution changes
	if (FMath::IsNearlyZero(SubtitleBoxX) || WasCanvasResized())
	{
		SubtitleBoxX = TargetSubtitleBoxX;
		SubtitleBoxW = TargetSubtitleBoxW;
	}

	SubtitleBoxX = FMath::FInterpTo(SubtitleBoxX, TargetSubtitleBoxX, GetWorld()->DeltaTimeSeconds, 15.f);
	SubtitleBoxW = FMath::FInterpTo(SubtitleBoxW, TargetSubtitleBoxW, GetWorld()->DeltaTimeSeconds, 15.f);

	DrawRect(BackgroundColor, SubtitleBoxX, SubtitlePositionY - SubtitlePaddingY, SubtitleBoxW, SubtitleHeight + (SubtitlePaddingY * 2.f));
	DrawText(SubtitleText, ShadowColor, SubtitlePositionX + 1.f, SubtitlePositionY + 1.f, SubtitleFont, SubtitleScale);
	DrawText(SubtitleText, TextColor, SubtitlePositionX, SubtitlePositionY, SubtitleFont, SubtitleScale);
}

// TODO: This is expensive, needs caching
const FString AEstPlayerHUD::GetHintKeyLabels() const
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

void AEstPlayerHUD::DrawReticule()
{
	const float ReticuleAlpha = Firearm.IsValid() ? FirearmReticuleAlpha : NormalReticuleAlpha;
	const float ReticuleSize = Firearm.IsValid() ? FirearmReticuleSize : NormalReticuleSize;
	UTexture* Reticule = Firearm.IsValid() ? FirearmReticule : NormalReticule;

	const float ReticuleAimingAlpha = Player->bIsAiming ? 0.f : 1.f;

	const FLinearColor ReticuleColor = HudColor.CopyWithNewOpacity(HudColor.A * ReticuleAlpha* ReticuleAimingAlpha);

	const float Size = FMath::Min(Canvas->SizeY * ReticuleSize, Canvas->SizeX * ReticuleSize);
	DrawTexture(Reticule, Canvas->SizeX * .5f - Size * .5f, Canvas->SizeY * .5f - Size * .5f, Size, Size, 0, 0, 1, 1, ReticuleColor);
}

void AEstPlayerHUD::DrawHealthIndicator()
{
	if (Player->HealthComponent->IsFrozen)
	{
		return;
	}

	const float Size = FMath::Min(Canvas->SizeY * HealthIndicatorSize, Canvas->SizeX * HealthIndicatorSize);
	const FVector2D Position = FVector2D(Canvas->SizeX * HealthIndicatorPosition.X - Size * .5f, Canvas->SizeY * HealthIndicatorPosition.Y - Size * .5f);

	const float HealthAlpha = FMath::Lerp(HealthDeadZone, 1.f - HealthDeadZone, Player->HealthComponent->GetAlpha());
	const float Offset = Size * (1 - HealthAlpha);
	const float Height = Size * HealthAlpha;

	DrawTexture(HealthIndicatorBackground, Position.X, Position.Y, Size, Size, 0, 0, 1, 1, HudColor);
	DrawTexture(HealthIndicatorForeground, Position.X, Position.Y + Offset, Size, Height, 0, 1.f - HealthAlpha, 1, HealthAlpha, HudColor);
}

void AEstPlayerHUD::DrawBatteryIndicator()
{
	if (Player->Flashlight->bHiddenInGame)
	{
		return;
	}

	const float Size = FMath::Min(Canvas->SizeY * BatteryIndicatorSize, Canvas->SizeX * BatteryIndicatorSize);
	const FVector2D Position = FVector2D(Canvas->SizeX * BatteryIndicatorPosition.X - Size * .5f, Canvas->SizeY * BatteryIndicatorPosition.Y - Size * .5f);

	const float BatteryAlpha = FMath::Lerp(BatteryDeadZone, 1.f - BatteryDeadZone, Player->Battery->GetAlpha());
	const float Offset = Size * (1 - BatteryAlpha);
	const float Height = Size * BatteryAlpha;
	const FLinearColor Tint = Player->Flashlight->bVisible ? FLinearColor(1.f, 1.f, 1.f, HudColor.A) : FLinearColor(.75, .75, .75, HudColor.A);

	DrawTexture(BatteryIndicatorBackground, Position.X, Position.Y, Size, Size, 0, 0, 1, 1, HudColor);
	DrawTexture(BatteryIndicatorForeground, Position.X, Position.Y + Offset, Size, Height, 0, 1.f - BatteryAlpha, 1, BatteryAlpha, Tint);
}

void AEstPlayerHUD::DrawDamageIndicators()
{
	if (Player->HealthComponent->IsFrozen)
	{
		return;
	}

	if (Player->HealthComponent->IsDepleted())
	{
		DrawRect(DeathOverlayColor, 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);
		return;
	}

	// Calculate the fade out
	const float FadeAlpha = 1.f - GetWorld()->TimeSince(LastDamageTime) / DamageIndicatorFadeTime;
	if (FadeAlpha < 0.f)
	{
		return;
	}

	if (LastDamageType == nullptr)
	{
		return;
	}

	const FLinearColor* Color = DamageTypeMapping.Find(LastDamageType->GetClass());
	if (Color == nullptr)
	{
		return;
	}

	const FLinearColor DamageColor = *Color;

	const FVector CameraForwardVector = Player->Camera->GetForwardVector();
	const float DotProduct = FVector::DotProduct(CameraForwardVector, LastDamageLocation);

	const float HalfX = Canvas->SizeX * .5f;
	const float HalfY = Canvas->SizeY * .5f;

	FVector Projected = Project(LastDamageLocation);

	Projected.X = FMath::Clamp(Projected.X, 0.f, float(Canvas->SizeX));
	Projected.Y = FMath::Clamp(Projected.Y, 0.f, float(Canvas->SizeY));

	const bool bIsFacing = Projected.Z > 0.f;
	if (!bIsFacing)
	{
		Projected.X -= Canvas->SizeX;
		Projected.Y -= Canvas->SizeY;
	}

	Projected.X = FMath::Abs(Projected.X);
	Projected.Y = FMath::Abs(Projected.Y);

	// Left
	const float LeftAlpha = LastDamageType->bCausedByWorld ? 1.f : 1.f - (FMath::Clamp(Projected.X, 0.f, HalfX) / HalfX);
	DrawTexture(DamageIndicator, 0, Canvas->SizeY, Canvas->SizeY, Canvas->SizeX * DamageIndicatorSize.X, 0, 0, 1, 1, DamageColor * LeftAlpha * FadeAlpha, BLEND_Additive, 1, false, 270.f);

	// Right
	const float RightAlpha = LastDamageType->bCausedByWorld ? 1.f : (FMath::Clamp(Projected.X, HalfX, (float)Canvas->SizeX) - HalfX) / HalfX;
	DrawTexture(DamageIndicator, Canvas->SizeX, 0, Canvas->SizeY, Canvas->SizeX * DamageIndicatorSize.X, 0, 0, 1, 1, DamageColor * RightAlpha * FadeAlpha, BLEND_Additive, 1, false, 90.f);

	// Top
	const float TopAlpha = LastDamageType->bCausedByWorld ? 1.f : 1.f - (FMath::Clamp(Projected.Y, 0.f, HalfY) / HalfY);
	DrawTexture(DamageIndicator, 0, 0, Canvas->SizeX, Canvas->SizeY * DamageIndicatorSize.Y, 0, 0, 1, 1, DamageColor * TopAlpha * FadeAlpha, BLEND_Additive, 1, false, 0);

	// Bottom
	const float BottomAlpha = LastDamageType->bCausedByWorld ? 1.f : (FMath::Clamp(Projected.Y, HalfY, (float)Canvas->SizeY) - HalfY) / HalfY;
	DrawTexture(DamageIndicator, Canvas->SizeX, Canvas->SizeY, Canvas->SizeX, Canvas->SizeY * DamageIndicatorSize.Y, 0, 0, 1, 1, DamageColor * BottomAlpha * FadeAlpha, BLEND_Additive, 1, false, 180.f);
}

void AEstPlayerHUD::HandleDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (DamagedActor->ActorHasTag(TAG_DEAD))
	{
		return;
	}

	LastDamageAmount = Damage;
	LastDamageLocation = DamageCauser == nullptr ? DamagedActor->GetActorLocation() : DamageCauser->GetActorLocation();
	LastDamageTime = GetWorld()->GetTimeSeconds();
	LastDamageType = DamageType;
}

void AEstPlayerHUD::HandleChangeWeapon(AEstBaseWeapon *Weapon)
{
	Firearm = Cast<AEstFirearmWeapon>(Weapon);
}

void AEstPlayerHUD::HandleShowHint(TArray<FName> Bindings, FText Label, bool bShowUntilHidden, FVector WorldLocation)
{
	HintBindings = Bindings;
	HintLabel = Label;
	HintWorldLocation = WorldLocation;
	HintFinishTime = GetWorld()->TimeSeconds + (bShowUntilHidden ? BIG_NUMBER : HintDuration);
}

void AEstPlayerHUD::HandleHideHint()
{
	HintFinishTime = GetWorld()->TimeSeconds;
}

void AEstPlayerHUD::HandleSetSubtitleText(const FText &SubtitleText)
{
	const bool bHasSubtitle = !SubtitleText.IsEmpty();
	const bool bDuringPaddingPeriod = GetWorld()->TimeSince(LastSubtitleTime) < 1.f;

 	bShouldDrawSubtitles = bHasSubtitle || bDuringPaddingPeriod;

	if (bHasSubtitle)
	{
		LastSubtitleText = SubtitleText;
		LastSubtitleTime = GetWorld()->TimeSeconds;
	}
}

bool AEstPlayerHUD::WasCanvasResized() const
{
	return Canvas->SizeX != LastCanvasSizeX || Canvas->SizeY != LastCanvasSizeY;
}
