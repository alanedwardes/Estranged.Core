// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EstGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API UEstGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	UFUNCTION(BlueprintPure, Category = UI)
	virtual bool GetMenuVisibleForever() { return bIsMenuVisibleForever; };

	UFUNCTION(BlueprintCallable, Category = UI)
	virtual void SetMenuVisibleForever(bool bNewIsMenuVisibleForever);

	UFUNCTION(BlueprintCallable, Category = UI)
	virtual void SetMenuVisibility(bool bNewIsMenuVisible);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = UI)
	TSubclassOf<class UUserWidget> MenuWidgetType;

	TSharedPtr<SWidget> GetRawMenuWidget() { return MenuSlateWidget; }

private:
	TSharedPtr<SWidget> MenuSlateWidget;
	bool bIsMenuVisibleForever;
};
