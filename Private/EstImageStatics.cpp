#include "EstCore.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "EstImageStatics.h"

bool UEstImageStatics::CompressImageToJpeg(const TArray<FColor> &SrcData, TArray<uint8> &DstData, int32 Width, int32 Height, int32 Quality)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	if (!ImageWrapper.IsValid())
	{
		return false;
	}

	if (!ImageWrapper->SetRaw(&SrcData[0], SrcData.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8))
	{
		return false;
	}

	DstData = ImageWrapper->GetCompressed(Quality);
	return true;
}

void UEstImageStatics::CropAndScaleImage(int32 SrcWidth, int32 SrcHeight, int32 DesiredWidth, int32 DesiredHeight, const TArray<FColor> &SrcData, TArray<FColor> &DstData)
{
	FImageUtils::CropAndScaleImage(SrcWidth, SrcHeight, DesiredWidth, DesiredHeight, SrcData, DstData);
}