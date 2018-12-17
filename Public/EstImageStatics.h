#pragma once

#include "EstImageStatics.generated.h"

UCLASS()
class ESTCORE_API UEstImageStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Images)
	static bool CompressImageToJpeg(const TArray<FColor> &SrcData, TArray<uint8> &DstData, int32 Width, int32 Height, int32 Quality);

	UFUNCTION(BlueprintCallable, Category = Images)
	static void CropAndScaleImage(int32 SrcWidth, int32 SrcHeight, int32 DesiredWidth, int32 DesiredHeight, const TArray<FColor> &SrcData, TArray<FColor> &DstData);
};
