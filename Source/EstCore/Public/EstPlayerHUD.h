#pragma once

#include "GameFramework/HUD.h"
#include "EstPlayerHUD.generated.h"

/** The Estranged player HUD */
UCLASS()
class ESTCORE_API AEstPlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	UTexture* DamageIndicator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	float DamageIndicatorFadeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	FVector2D DamageIndicatorSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	TMap<TSubclassOf<UDamageType>, FLinearColor> DamageTypeMapping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	FLinearColor DeathOverlayColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reticule)
	float NormalReticuleAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reticule)
	UTexture* NormalReticule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reticule)
	float NormalReticuleSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reticule)
	float FirearmReticuleAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reticule)
	UTexture* FirearmReticule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reticule)
	float FirearmReticuleSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Status)
	FVector2D StatusIndicatorOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Status)
	float StatusIndicatorSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Status)
	float StatusIndicatorSpacing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
	UTexture* HealthIndicatorForeground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
	UTexture* HealthIndicatorBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
	float HealthDeadZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Battery)
	UTexture* BatteryIndicatorForeground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Battery)
	UTexture* BatteryIndicatorBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Battery)
	float BatteryDeadZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Oxygen)
	UTexture* OxygenIndicatorForeground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Oxygen)
	UTexture* OxygenIndicatorBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Oxygen)
	float OxygenDeadZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Build)
	UFont* BuildLabelFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Build)
	FLinearColor BuildLabelColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Build)
	FVector2D BuildLabelPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hint)
	UFont* HintKeyFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hint)
	UFont* HintLabelFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hint)
	FVector2D HintLabelPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hint)
	FVector2D HintLabelPadding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hint)
	FLinearColor HintLabelColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hint)
	float HintDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo)
	UFont* AmmoLabelRoundsFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo)
	UFont* AmmoLabelMagazinesFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo)
	FVector2D AmmoLabelPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	FText LoadingLabelText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	UFont* LoadingLabelFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loading)
	bool bIsLoading;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = General)
	bool bDisableHUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = General)
	bool bEnableStatsForNerds;

	UFUNCTION()
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void DrawHUD() override;

	UFUNCTION()
	virtual void DrawGameSpecificHUD();

	UFUNCTION()
	virtual void HandleDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	virtual void HandleChangeWeapon(AEstBaseWeapon* Weapon);

	UFUNCTION()
	virtual void HandleShowHint(TArray<FName> Bindings, FText Label, bool bShowUntilHidden, FVector WorldLocation);

	UFUNCTION()
	virtual void HandleHideHint();
protected:
	UFUNCTION()
	virtual bool ShouldDrawHUD();

	UFUNCTION()
	virtual void DrawReticule();

	UFUNCTION()
	virtual float DrawHealthIndicator(float LeftOffset);

	UFUNCTION()
	virtual float DrawBatteryIndicator(float LeftOffset);

	UFUNCTION()
	virtual float DrawOxygenIndicator(float LeftOffset);

	UFUNCTION()
	virtual void DrawDamageIndicators();

	UFUNCTION()
	virtual void DrawAmmoLabels();

	UFUNCTION()
	virtual void DrawHint();

	UFUNCTION()
	virtual void DrawLoadingIndicator();

	UFUNCTION()
	virtual const FString GetHintKeyLabels() const;

	UPROPERTY()
	TWeakObjectPtr<class AEstPlayer> Player;

	UPROPERTY()
	TWeakObjectPtr<class AEstPlayerController> Controller;

	UPROPERTY()
	TWeakObjectPtr<class AEstFirearmWeapon> Firearm;

	UPROPERTY()
	const UDamageType* LastDamageType;

	UPROPERTY()
	float LastDamageAmount;

	UPROPERTY()
	FVector LastDamageLocation;

	UPROPERTY()
	float LastDamageTime;

	UPROPERTY()
	float LastOxygenAlpha;

	UPROPERTY()
	FLinearColor HudColor = FLinearColor(1.f, 1.f, 1.f, 0.f);

	UPROPERTY()
	TArray<FName> HintBindings;

	UPROPERTY()
	FText HintLabel;

	UPROPERTY()
	FVector HintWorldLocation;

	UPROPERTY()
	float HintFinishTime;

	UPROPERTY()
	int32 LastCanvasX;

	UPROPERTY()
	int32 LastCanvasSizeX;

	UPROPERTY()
	int32 LastCanvasSizeY;
};
