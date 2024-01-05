// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstScreenshotTaker.h"
#include "EstCore.h"
#include "Gameplay/EstGameplayStatics.h"
#include "Gameplay/EstGameInstance.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/Engine.h"

void UEstScreenshotTaker::RequestScreenshot()
{
	if (GEngine == nullptr || GEngine->GameViewport == nullptr || bIsScreenshotRequested)
	{
		return;
	}

	if (UEstGameplayStatics::GetEstGameInstance(this)->GetLoggerEnabled())
	{
		UEstGameplayStatics::GetEstGameInstance(this)->LogMessage(FEstLoggerMessage(this, EEstLoggerLevel::Trace, TEXT("Screenshot requested")));
	}

	bIsScreenshotRequested = true;
	GEngine->GameViewport->OnScreenshotCaptured().AddUObject(this, &UEstScreenshotTaker::AcceptScreenshot);

	FScreenshotRequest::RequestScreenshot(false);
}

void UEstScreenshotTaker::AcceptScreenshot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData)
{
	OnScreenshot.Broadcast(InImageData, InSizeX, InSizeY);
	
	if (UEstGameplayStatics::GetEstGameInstance(this)->GetLoggerEnabled())
	{
		UEstGameplayStatics::GetEstGameInstance(this)->LogMessage(FEstLoggerMessage(this, EEstLoggerLevel::Trace, TEXT("Screenshot delivered")));
	}

	GEngine->GameViewport->OnScreenshotCaptured().RemoveAll(this);
	bIsScreenshotRequested = false;
}
