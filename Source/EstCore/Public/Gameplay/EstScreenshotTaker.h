// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "EstScreenshotTaker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScreenshot, const TArray<FColor>&, InImageData, int32, InSizeX, int32, InSizeY);

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
	FOnScreenshot OnScreenshot;
protected:
	virtual void AcceptScreenshot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData);

	bool bIsScreenshotRequested;
};
