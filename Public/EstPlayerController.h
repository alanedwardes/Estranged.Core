#pragma once

#include "GameFramework/PlayerController.h"
#include "EstPlayerController.generated.h"

/** The Estranged player controller.  */
UCLASS(abstract, BlueprintType)
class ESTCORE_API AEstPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void Destroyed() override;

	virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;

	virtual bool InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad) override;

	virtual void ReceivedPlayer() override;

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bIsUsingGamepad;

	UFUNCTION(BlueprintCallable, Category = "Menu")
	virtual void SetMenuVisible(bool bNewIsVisible);

	UPROPERTY(BlueprintReadOnly, Category = "Menu")
	bool bIsMenuVisible;

	/** The menu widget to show when paused */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<class UUserWidget> MenuWidgetType;

	UPROPERTY(Transient, DuplicateTransient)
	class UUserWidget* MenuWidget;

	TSharedPtr<class SWidget> MenuSlateWidgetInstance;
};
