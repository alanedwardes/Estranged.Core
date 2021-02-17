// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"
#include "UI/EstMenuSection.h"
#include "UI/EstMenuModal.h"
#include "Engine/StreamableManager.h"
#include "EstMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResumeGame);

UCLASS(abstract)
class ESTCORE_API UEstMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent)
	void ShowMenu(FName RedirectToMenu);

	UFUNCTION(BlueprintNativeEvent)
	void LoadLevel(FName LevelName);

	UFUNCTION(BlueprintCallable)
	virtual void Action(FEstMenuAction Action);

	UFUNCTION(BlueprintCallable)
	virtual void AsyncNavigate(TSoftClassPtr<UEstMenuSection> MenuSection, FName Context);

	UFUNCTION(BlueprintCallable)
	virtual void Navigate(UEstMenuSection* MenuSection, FName Context);

	UFUNCTION(BlueprintCallable)
	virtual void RemoveMenu();

	UFUNCTION(BlueprintCallable)
	virtual void FocusMenu();

	UFUNCTION(BlueprintCallable)
	virtual void AsyncModal(TSoftClassPtr<UEstMenuModal> MenuModal, FName Context);

	UFUNCTION(BlueprintCallable)
	virtual void Modal(UEstMenuModal* MenuModal, FName Context);

	UFUNCTION(BlueprintCallable)
	virtual void RemoveModal();

	UFUNCTION(BlueprintCallable)
	virtual void ExitModal();

	UFUNCTION(BlueprintCallable)
	virtual void AsyncExtra(TSoftClassPtr<UUserWidget> ExtraSection);

	UFUNCTION(BlueprintCallable)
	virtual void Extra(UUserWidget* ExtraSection);

	UFUNCTION(BlueprintCallable)
	virtual void RemoveExtra();

	UFUNCTION(BlueprintCallable)
	virtual void SetMenuLoadingSpinnerVisibile(bool bIsVisible);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPanelWidget* MenuSectionContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UEstMenuSection* CurrentMenuSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidget* MenuLoadingSpinner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UOverlay* MenuModalContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* NavigationSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UEstMenuModal* CurrentMenuModal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPanelWidget* MenuExtraContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* CurrentExtraSection;

protected:
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

protected:
	virtual void ResumeGame();
};
