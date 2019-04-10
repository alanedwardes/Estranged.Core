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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Build)
	UFont* BuildLabelFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Build)
	FLinearColor BuildLabelColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Build)
	FVector2D BuildLabelPosition;

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
protected:
	UFUNCTION()
	virtual bool ShouldDrawHUD();

	UFUNCTION()
	virtual void DrawDamageIndicators();

	UFUNCTION()
	virtual void DrawLoadingIndicator();

	UPROPERTY()
	TWeakObjectPtr<class AEstPlayer> Player;

	UPROPERTY()
	TWeakObjectPtr<class AEstPlayerController> Controller;

	UPROPERTY()
	const UDamageType* LastDamageType;

	UPROPERTY()
	float LastDamageAmount;

	UPROPERTY()
	FVector LastDamageLocation;

	UPROPERTY()
	float LastDamageTime;

	UPROPERTY()
	FLinearColor HudColor = FLinearColor(1.f, 1.f, 1.f, 0.f);

	UPROPERTY()
	int32 LastCanvasX;

	UPROPERTY()
	int32 LastCanvasSizeX;

	UPROPERTY()
	int32 LastCanvasSizeY;
};
