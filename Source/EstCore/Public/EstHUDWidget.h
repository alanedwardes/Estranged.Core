// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EstHUDWidget.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FEstSubtitle
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SubtitleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SubtitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsClosedCaption;

	bool bIsHidden;
};

UCLASS(abstract)
class ESTCORE_API UEstHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct();

	virtual void NativeDestruct();

	virtual void HandleSetSubtitleText(const FText & SubtitleText);

public:
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
	UFUNCTION(BlueprintPure)
	bool GetEnableClosedCaptions() { return bEnableClosedCaptions || bForceClosedCaptions; };

	UFUNCTION(BlueprintPure)
	bool GetEnableSubtitles() { return !bDisableSubtitles; };

	UPROPERTY()
	bool bDisableSubtitles;

	UPROPERTY()
	bool bEnableClosedCaptions;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bForceClosedCaptions;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FText LastSubtitleText;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FEstSubtitle> Subtitles;

	UFUNCTION(BlueprintImplementableEvent, Category = Subtitles)
	void OnShowSubtitle(FEstSubtitle Subtitle);

	UFUNCTION(BlueprintImplementableEvent, Category = Subtitles)
	void OnHideSubtitle(FEstSubtitle Subtitle);

	UFUNCTION(BlueprintImplementableEvent, Category = Subtitles)
	void OnDestroySubtitle(FEstSubtitle Subtitle);
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
private:
	virtual void NewSubtitle(const FText &SubtitleText);
};
