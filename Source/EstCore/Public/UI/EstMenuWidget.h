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
	UFUNCTION(BlueprintImplementableEvent)
	void OnShowMenu(FName RedirectToMenu);

	UFUNCTION(BlueprintImplementableEvent)
	void OnHideMenu();

	UFUNCTION(BlueprintImplementableEvent)
	void OnLoadLevelByReference(const TSoftObjectPtr<UWorld> &Level);

	UFUNCTION(BlueprintImplementableEvent)
	void OnLoadCheckpoint(struct FEstCheckpoint Checkpoint);

	UFUNCTION(BlueprintImplementableEvent)
	void OnMenuLoadingStateChanged(bool bIsLoading);

	UFUNCTION(BlueprintImplementableEvent)
	void OnResumeGame();

	UFUNCTION(BlueprintCallable)
	virtual void Action(FEstMenuAction MenuAction);

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
	virtual void ResumeGame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPanelWidget* MenuSectionContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UEstMenuSection* CurrentMenuSection;

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
};
