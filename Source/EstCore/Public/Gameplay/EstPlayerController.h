#pragma once

#include "GameFramework/PlayerController.h"
#include "EstPlayerController.generated.h"

/** The Estranged player controller.  */
UCLASS(abstract, BlueprintType)
class ESTCORE_API AEstPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AEstPlayerController(const class FObjectInitializer& PCIP);

	virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;

	virtual bool InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad) override;

	virtual void BeginDestroy() override;

	UPROPERTY(BlueprintReadOnly, Category = Input)
	bool bIsUsingGamepad;

	UFUNCTION(BlueprintCallable, Category = Input)
	virtual void SetMenuFocusState(bool bNewIsFocussed);
};
