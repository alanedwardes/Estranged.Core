// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "EstWebImageDecorator.generated.h"

/**
 * Lazy-loads images from HTTP endpoints into a URichTextBlock.
 *
 * Handles <img url="..." width="64" height="64"/> tags. The download begins
 * the first time the widget is painted (viewport-culling aware), and the
 * placeholder fades out over FadeDuration seconds once it completes.
 */
UCLASS(Blueprintable)
class ESTCORE_API UEstWebImageDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()

public:
	UEstWebImageDecorator(const FObjectInitializer& ObjectInitializer);

	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;

	/** Fills the widget area behind the spinner while loading. */
	UPROPERTY(EditAnywhere, Category = Appearance)
	FSlateBrush BackgroundPlaceholderBrush;

	/** Shown centred over the background while loading (e.g. a spinner). */
	UPROPERTY(EditAnywhere, Category = Appearance)
	FSlateBrush ForegroundPlaceholderBrush;

	/** Cross-fade duration in seconds once the download completes. */
	UPROPERTY(EditAnywhere, Category = Appearance, meta = (ClampMin = "0.0"))
	float FadeDuration = 0.25f;
};
