// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EstHUDWidget.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FEstSubtitle
{
	GENERATED_BODY()

	FEstSubtitle() : bIsClosedCaption(false) {}

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGuid SubtitleId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText SubtitleText;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
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

	// Begin weapon
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TWeakObjectPtr<class AEstFirearmWeapon> Firearm;

	UFUNCTION()
	void HandleChangeWeapon(class AEstBaseWeapon *Weapon);

	UFUNCTION(BlueprintNativeEvent, Category = General)
	void OnChangePlayer(class AEstPlayer* OldPlayer, class AEstPlayer* NewPlayer);

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION(BlueprintCallable)
	const FString GetClipLabel() const;
	
	UFUNCTION(BlueprintCallable)
	const FString GetAmmoLabel() const;
	// End weapon

	// Begin subtitles
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

	UFUNCTION(BlueprintCallable)
	virtual void ShowHint(TArray<class UInputAction*> Bindings, FText Label, float ShowTime, FVector WorldLocation);

	UFUNCTION(BlueprintCallable)
	virtual void HideHint();

	UPROPERTY()
	TArray<class UInputAction*> HintBindings;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FText HintLabel;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FVector HintWorldLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float HintFinishTime;

	UFUNCTION(BlueprintCallable)
	const TSet<FKey> GetHintKeys() const;
	// End hints
private:
	virtual void NewSubtitle(const FText &SubtitleText);
};
