// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "EstScreenshotTaker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenshotJpeg, const TArray<uint8>&, Jpeg);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class UEstScreenshotTaker : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Screenshot)
	virtual void RequestScreenshot();

	UPROPERTY(BlueprintAssignable, Category = Screenshot)
	FOnScreenshotJpeg OnScreenshotJpeg;
protected:
	virtual void AcceptScreenshot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData);

	bool bIsScreenshotRequested;
};
