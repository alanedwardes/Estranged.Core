// Estranged is a trade mark of Alan Edwardes.

#include "EstCore.h"
#include "EstScreenshotTaker.h"
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
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	if (!ImageWrapper.IsValid())
	{
		bIsScreenshotRequested = false;
		return;
	}

	if (!ImageWrapper->SetRaw(&InImageData[0], InImageData.Num() * sizeof(FColor), InSizeX, InSizeY, ERGBFormat::BGRA, 8))
	{
		bIsScreenshotRequested = false;
		return;
	}

	const TArray<uint8>& CompressedImage = ImageWrapper->GetCompressed(90);

	OnScreenshotJpeg.Broadcast(CompressedImage);

	GEngine->GameViewport->OnScreenshotCaptured().RemoveAll(this);
	bIsScreenshotRequested = false;
}
