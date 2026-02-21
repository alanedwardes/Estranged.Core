// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "EstLinkDecorator.generated.h"

/**
 * Handles <a> tags in a URichTextBlock.
 * Valid attributes: url, href.
 * Example: <a url="https://example.com">Click me</a>
 */
UCLASS(Blueprintable)
class ESTCORE_API UEstLinkDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()

public:
	UEstLinkDecorator(const FObjectInitializer& ObjectInitializer);

	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;
};
