// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "Blueprint/UserWidget.h"
#include "EstMenuBase.generated.h"

UCLASS(Abstract)
class ESTCORE_API UEstMenuBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnBack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Context;

protected:
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidget* LandingWidget;

private:
	FKeyEvent PressedKey;
};
