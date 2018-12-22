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
		DisableTemporalAntiAliasing = true; // Default to TXAA off
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
	virtual bool GetEnableStatsForNerds() { return EnableStatsForNerds; }

	UFUNCTION(BlueprintCallable)
	virtual void SetEnableStatsForNerds(bool NewEnableStatsForNerds) { EnableStatsForNerds = NewEnableStatsForNerds; }

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
	virtual bool GetDisableTemporalAntiAliasing()
	{ 
#if PLATFORM_LINUX
		// On Linux, only permit disabling TXAA if we're not using the forward renderer
		static IConsoleVariable* CVarForwardShading = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForwardShading"));
		return CVarForwardShading->GetInt() == 0 ? DisableTemporalAntiAliasing : false;
#else
		return DisableTemporalAntiAliasing;
#endif
	}

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
	bool DisableHighlight;

	UPROPERTY()
	bool DisableDepthOfField;

	UPROPERTY()
	bool DisableMotionBlur;

	UPROPERTY()
	bool DisableVignette;
};
