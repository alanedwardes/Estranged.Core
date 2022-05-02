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

	virtual bool InputKey(const FInputKeyParams& Params) override;

	virtual void BeginDestroy() override;

	UPROPERTY(BlueprintReadOnly, Category = Input)
	bool bIsUsingGamepad;

	UFUNCTION(BlueprintCallable, Category = Input)
	virtual void SetMenuFocusState(bool bNewIsFocussed);
};
