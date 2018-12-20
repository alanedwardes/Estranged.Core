// Estranged is a trade mark of Alan Edwardes.

#include "EstScreenshotTaker.h"
#include "EstCore.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

void UEstScreenshotTaker::RequestScreenshot()
{
	if (GEngine == nullptr || GEngine->GameViewport == nullptr || bIsScreenshotRequested)
	{
		return;
	}

	bIsScreenshotRequested = true;
	GEngine->GameViewport->OnScreenshotCaptured().AddUObject(this, &UEstScreenshotTaker::AcceptScreenshot);

	FScreenshotRequest::RequestScreenshot(false);
}

void UEstScreenshotTaker::AcceptScreenshot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData)
{
	OnScreenshot.Broadcast(InImageData, InSizeX, InSizeY);

	GEngine->GameViewport->OnScreenshotCaptured().RemoveAll(this);
	bIsScreenshotRequested = false;
}
