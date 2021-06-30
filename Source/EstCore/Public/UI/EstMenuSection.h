// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "UI/EstMenuBase.h"
#include "EstMenuSection.generated.h"

UENUM(BlueprintType)
enum EEstMenuAction
{
	ExitGame,
	TransitionToLevel,
	ResumeGame
};

USTRUCT(BlueprintType)
struct FEstMenuAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EEstMenuAction> Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> Level;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModal, TSoftClassPtr<UEstMenuModal>, Modal, FName, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNavigate, TSoftClassPtr<UEstMenuSection>, Section, FName, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAction, FEstMenuAction, Action);

UCLASS(abstract)
class ESTCORE_API UEstMenuSection : public UEstMenuBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FOnNavigate OnNavigate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FOnModal OnModal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FOnAction OnAction;

	UFUNCTION(BlueprintPure)
	virtual ESlateVisibility VisibleOnDesktop() const;
};
