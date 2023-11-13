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

	virtual void SetupInputComponent() override;

	virtual void ShowMainMenu() { ShowMenuSection(NAME_None); };

	virtual void ShowMenuSection(FName MenuSection);

	virtual void ShowConsoleMenu() { ShowMenuSection("Console"); }

	virtual void ShowLoadGameMenu() { ShowMenuSection("LoadGame"); }

	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* MainMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ConsoleAction;

	UPROPERTY(BlueprintReadOnly, Category = Input)
	bool bIsUsingGamepad;

	UFUNCTION(BlueprintCallable, Category = Input)
	virtual void SetMenuFocusState(bool bNewIsFocussed);
};
