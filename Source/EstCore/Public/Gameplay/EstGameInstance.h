// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Sound/EstMusic.h"
#include "UI/EstLoggerWidget.h"
#include "EstGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FEstMenuVisibilityContext
{
	GENERATED_BODY()

	FEstMenuVisibilityContext()
		: FEstMenuVisibilityContext(false)
	{
	}

	FEstMenuVisibilityContext(bool bInIsMenuVisible)
		: FEstMenuVisibilityContext(bInIsMenuVisible, NAME_None)
	{
	}

	FEstMenuVisibilityContext(bool bInIsMenuVisible, FName InRedirectToMenu)
		: FEstMenuVisibilityContext(bInIsMenuVisible, false, InRedirectToMenu)
	{
	}

	FEstMenuVisibilityContext(bool bInIsMenuVisible, bool bInIsMenuVisibleForever)
		: FEstMenuVisibilityContext(bInIsMenuVisible, bInIsMenuVisibleForever, NAME_None)
	{
	}

	FEstMenuVisibilityContext(bool bInIsMenuVisible, bool bInIsMenuVisibleForever, FName InMenuSection)
	{
		bIsMenuVisible = bInIsMenuVisible;
		bIsMenuVisibleForever = bInIsMenuVisibleForever;
		MenuSection = InMenuSection;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMenuVisibleForever;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMenuVisible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MenuSection;
};

UCLASS()
class ESTCORE_API UEstGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	virtual void Shutdown() override;

	virtual void OnStart() override;

	virtual void PreLoadMap(const FString & InMapName);
	virtual void PostLoadMapWithWorld(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = Music)
	virtual void FadeMusic();

	UFUNCTION(BlueprintCallable, Category = Music)
	virtual void StopMusic();

	UFUNCTION(BlueprintCallable, Category = Music)
	virtual void PlayMusic(FEstMusic Music);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Music)
	class USoundClass* MusicSoundClass;

	UFUNCTION(BlueprintPure, Category = Music)
	virtual FEstMusic GetMusic();

	UFUNCTION(BlueprintPure, Category = UI)
	virtual bool GetMenuVisibleForever() { return VisibilityContext.bIsMenuVisibleForever; };

	UFUNCTION(BlueprintCallable, Category = UI)
	virtual void SetMenuVisibility(FEstMenuVisibilityContext InVisibilityContext);

	UFUNCTION(BlueprintCallable, Category = UI)
	virtual void LogMessage(FEstLoggerMessage Message);

	UFUNCTION(BlueprintCallable, Category = UI)
	virtual void RefreshLoggerState();

	virtual void SetLoggerEnabled(bool NewIsEnabled);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = UI)
	TSubclassOf<class UUserWidget> MenuWidgetType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = UI)
	TSubclassOf<class UUserWidget> LoggerWidgetType;

	TSharedPtr<SWidget> GetRawMenuWidget() { return MenuSlateWidget; }
private:
	class UAudioComponent* AudioComponent;

	virtual float GetPlayPosition();
	virtual void PlayMusicInternal(FEstMusic Music);
	virtual void SetLoggerVisible(bool NewIsVisible);
	virtual bool Tick(float DeltaTime);
	bool LazilyCreateAudioComponent();

	FDelegateHandle TickDelegateHandle;
	TOptional<FEstMusic> NextMusic;
	float MusicStartTime;
	float GameInstanceTime;
	float MusicFadeCompleteTime;
	bool bWasFadingOut;

	TSharedPtr<SWidget> LoggerSlateWidget;
	class UEstLoggerWidget* LoggerUserWidget;
	bool bIsLoggerVisible;
	bool bIsLoggerEnabled;

	TSharedPtr<SWidget> MenuSlateWidget;
	class UEstMenuWidget* MenuUserWidget;
	FEstMenuVisibilityContext VisibilityContext;
};
