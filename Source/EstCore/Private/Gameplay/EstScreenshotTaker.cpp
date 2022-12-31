// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstScreenshotTaker.h"
#include "EstCore.h"
#include "Gameplay/EstGameplayStatics.h"
#include "Gameplay/EstGameInstance.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "EngineGlobals.h"

void UEstScreenshotTaker::RequestScreenshot()
{
	if (GEngine == nullptr || GEngine->GameViewport == nullptr || bIsScreenshotRequested)
	{
		return;
	}

	UEstGameplayStatics::GetEstGameInstance(this)->LogMessage(FEstLoggerMessage(this, EEstLoggerLevel::Trace, FText::FromString("Screenshot requested")));

	bIsScreenshotRequested = true;
	GEngine->GameViewport->OnScreenshotCaptured().AddUObject(this, &UEstScreenshotTaker::AcceptScreenshot);

	FScreenshotRequest::RequestScreenshot(false);
}

void UEstScreenshotTaker::AcceptScreenshot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData)
{
	OnScreenshot.Broadcast(InImageData, InSizeX, InSizeY);

	UEstGameplayStatics::GetEstGameInstance(this)->LogMessage(FEstLoggerMessage(this, EEstLoggerLevel::Trace, FText::FromString("Screenshot delivered")));

	GEngine->GameViewport->OnScreenshotCaptured().RemoveAll(this);
	bIsScreenshotRequested = false;
}
