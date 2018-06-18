// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EstGameMode.generated.h"

UCLASS()
class ESTCORE_API AEstGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = UI)
	virtual void SetMenuVisibleForever() { bIsVisibleForever = true; SetMenuVisibility(true); }

	UFUNCTION(BlueprintCallable, Category = UI)
	virtual void SetMenuVisibility(bool bNewIsVisible);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = UI)
	TSubclassOf<class UUserWidget> MenuWidgetType;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = UI)
	bool bIsVisibleForever;

	TSharedPtr<SWidget> GetRawWidget() { return MenuSlateWidget; }

private:
	TSharedPtr<SWidget> MenuSlateWidget;
};
