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
};
