// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EstHUDWidget.generated.h"

UCLASS(abstract)
class ESTCORE_API UEstHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct();

	virtual void NativeDestruct();

	virtual void HandleSetSubtitleText(const FText & SubtitleText);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = General)
	bool bDisableHUD;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TWeakObjectPtr<class AEstPlayer> Player;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TWeakObjectPtr<class AEstPlayerController> Controller;

	// Begin stats
	UFUNCTION(BlueprintCallable)
	const FString GetStatsForNerds() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = General)
	bool bEnableStatsForNerds;
	// End stats

	// Begin weapon
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TWeakObjectPtr<class AEstFirearmWeapon> Firearm;

	UFUNCTION()
	void HandleChangeWeapon(class AEstBaseWeapon *Weapon);

	UFUNCTION(BlueprintCallable)
	const FString GetClipLabel() const;
	
	UFUNCTION(BlueprintCallable)
	const FString GetAmmoLabel() const;
	// End weapon

	// Begin subtitles
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bDisableSubtitles;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float LastSubtitleTime;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bShouldDrawSubtitles;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FText LastSubtitleText;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float SubtitleAlpha;
	// End subtitles

	// Begin hints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hint)
	float HintDuration;

	UFUNCTION()
	virtual void HandleShowHint(TArray<FName> Bindings, FText Label, bool bShowUntilHidden, FVector WorldLocation);

	UFUNCTION()
	virtual void HandleHideHint();

	UPROPERTY()
	TArray<FName> HintBindings;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FText HintLabel;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FVector HintWorldLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float HintFinishTime;

	UFUNCTION(BlueprintCallable)
	const FString GetHintKeyLabels() const;

	UFUNCTION(BlueprintCallable)
	const TSet<FKey> GetHintKeys() const;
	// End hints
};
