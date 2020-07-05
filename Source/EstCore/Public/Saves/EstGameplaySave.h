// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EstPlayer.h"
#include "EstSave.h"
#include "EstGameplaySave.generated.h"

UCLASS()
class ESTCORE_API UEstGameplaySave : public UEstSave
{
	GENERATED_BODY()
	
public:
	UEstGameplaySave()
	{
		FieldOfView = 90.f;
		Gamma = 1.0;
		DisableMotionBlur = true; // Default motion blur off
#if PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC
		DisableTemporalAntiAliasing = true; // Default to TXAA off on desktop
#else
		DisableTemporalAntiAliasing = false; // Default to TXAA on everywhere else
#endif
		SubtitleFontSizeMultiplier = 1.f;
	}

	virtual FString GetSlotName() { return  TEXT("Gameplay"); };

	UFUNCTION(BlueprintCallable)
	virtual float GetFieldOfView() { return FieldOfView; }

	UFUNCTION(BlueprintCallable)
	virtual void SetFieldOfView(float NewFieldOfView) { FieldOfView = FMath::Clamp(NewFieldOfView, 70.f, 120.f); }

	UFUNCTION(BlueprintCallable)
	virtual float GetGamma() { return Gamma; }

	UFUNCTION(BlueprintCallable)
	virtual void SetGamma(float NewGamma) { Gamma = NewGamma; }

	UFUNCTION(BlueprintCallable)
	virtual bool GetEnableCommentary() { return EnableCommentary; }

	UFUNCTION(BlueprintCallable)
	virtual void SetEnableCommentary(bool NewEnableCommentary) { EnableCommentary = NewEnableCommentary; }

	UFUNCTION(BlueprintCallable)
	virtual bool GetEnableStatsForNerds() { return EnableStatsForNerds; }

	UFUNCTION(BlueprintCallable)
	virtual void SetEnableStatsForNerds(bool NewEnableStatsForNerds) { EnableStatsForNerds = NewEnableStatsForNerds; }

	UFUNCTION(BlueprintCallable)
	virtual bool GetEnableDebugLog() { return EnableDebugLog; }

	UFUNCTION(BlueprintCallable)
	virtual void SetEnableDebugLog(bool NewEnableDebugLog) { EnableDebugLog = NewEnableDebugLog; }

	UFUNCTION(BlueprintCallable)
	virtual bool GetDisableHUD() { return DisableHUD; }

	UFUNCTION(BlueprintCallable)
	virtual void SetDisableHUD(bool NewDisableHUD) { DisableHUD = NewDisableHUD; }

	UFUNCTION(BlueprintCallable)
	virtual bool GetDisableHighlight() { return DisableHighlight; }

	UFUNCTION(BlueprintCallable)
	virtual void SetDisableHighlight(bool NewDisableHighlight) { DisableHighlight = NewDisableHighlight; }

	UFUNCTION(BlueprintCallable)
	virtual bool GetDisableDepthOfField() { return DisableDepthOfField; }

	UFUNCTION(BlueprintCallable)
	virtual void SetDisableDepthOfField(bool NewDisableDepthOfField) { DisableDepthOfField = NewDisableDepthOfField; }

	UFUNCTION(BlueprintCallable)
	virtual bool GetDisableTemporalAntiAliasing() { return DisableTemporalAntiAliasing; }

	UFUNCTION(BlueprintCallable)
	virtual void SetDisableTemporalAntiAliasing(bool NewDisableTemporalAntiAliasing) { DisableTemporalAntiAliasing = NewDisableTemporalAntiAliasing; };

	UFUNCTION(BlueprintCallable)
	virtual bool GetDisableMotionBlur() { return DisableMotionBlur; }

	UFUNCTION(BlueprintCallable)
	virtual void SetDisableMotionBlur(bool NewDisableMotionBlur) { DisableMotionBlur = NewDisableMotionBlur; };

	UFUNCTION(BlueprintCallable)
	virtual bool GetDisableVignette() { return DisableVignette; }

	UFUNCTION(BlueprintCallable)
	virtual void SetDisableVignette(bool NewDisableVignette) { DisableVignette = NewDisableVignette; };

	UFUNCTION(BlueprintCallable)
	virtual bool GetDisableAmbientOcclusion() { return DisableAmbientOcclusion; }

	UFUNCTION(BlueprintCallable)
	virtual void SetDisableAmbientOcclusion(bool NewDisableAmbientOcclusion) { DisableAmbientOcclusion = NewDisableAmbientOcclusion; };

	UFUNCTION(BlueprintCallable)
	virtual bool GetDisableSubtitles() { return DisableSubtitles; }

	UFUNCTION(BlueprintCallable)
	virtual void SetDisableSubtitles(bool NewDisableSubtitles) { DisableSubtitles = NewDisableSubtitles; };

	UFUNCTION(BlueprintCallable)
	virtual bool GetEnableClosedCaptions() { return EnableClosedCaptions; };

	UFUNCTION(BlueprintCallable)
	virtual void SetEnableClosedCaptions(bool NewEnableClosedCaptions) { EnableClosedCaptions = NewEnableClosedCaptions; };

	UFUNCTION(BlueprintCallable)
	virtual bool GetUseSimpleSubtitleFont() { return UseSimpleSubtitleFont; };

	UFUNCTION(BlueprintCallable)
	virtual void SetUseSimpleSubtitleFont(bool NewUseSimpleSubtitleFont) { UseSimpleSubtitleFont = NewUseSimpleSubtitleFont; };

	UFUNCTION(BlueprintCallable)
	virtual float GetSubtitleFontSizeMultiplier() { return SubtitleFontSizeMultiplier; };

	UFUNCTION(BlueprintCallable)
	virtual void SetSubtitleFontSizeMultiplier(float NewSubtitleFontSizeMultiplier) { SubtitleFontSizeMultiplier = NewSubtitleFontSizeMultiplier; };

private:
	UPROPERTY()
	float FieldOfView;

	UPROPERTY()
	float Gamma;

	UPROPERTY()
	bool DisableTemporalAntiAliasing;

	UPROPERTY()
	bool DisableHUD;

	UPROPERTY()
	bool EnableStatsForNerds;

	UPROPERTY()
	bool EnableDebugLog;

	UPROPERTY()
	bool EnableCommentary;

	UPROPERTY()
	bool DisableHighlight;

	UPROPERTY()
	bool DisableDepthOfField;

	UPROPERTY()
	bool DisableMotionBlur;

	UPROPERTY()
	bool DisableVignette;

	UPROPERTY()
	bool DisableAmbientOcclusion;

	UPROPERTY()
	bool DisableSubtitles;

	UPROPERTY()
	bool EnableClosedCaptions;

	UPROPERTY()
	bool UseSimpleSubtitleFont;

	UPROPERTY()
	float SubtitleFontSizeMultiplier;
};
