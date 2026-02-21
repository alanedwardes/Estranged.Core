// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstLinkDecorator.h"

#include "Components/RichTextBlock.h"
#include "Framework/Text/ITextDecorator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "HAL/PlatformProcess.h"

#include "Framework/Text/SlateWidgetRun.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Styling/CoreStyle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EstLinkDecorator)

class FEstLinkDecorator : public ITextDecorator
{
public:
	FEstLinkDecorator(URichTextBlock* InOwner, UEstLinkDecorator* InDecorator)
		: Owner(InOwner)
		, Decorator(InDecorator)
	{
	}

	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override
	{
		return RunParseResult.Name == TEXT("a");
	}

	virtual TSharedRef<ISlateRun> Create(const TSharedRef<class FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) override
	{
		FTextRunInfo RunInfo(RunParseResult.Name, FText::FromString(OriginalText.Mid(RunParseResult.ContentRange.BeginIndex, RunParseResult.ContentRange.EndIndex - RunParseResult.ContentRange.BeginIndex)));
		for (const TPair<FString, FTextRange>& Pair : RunParseResult.MetaData)
		{
			RunInfo.MetaData.Add(Pair.Key, OriginalText.Mid(Pair.Value.BeginIndex, Pair.Value.EndIndex - Pair.Value.BeginIndex));
		}

		FString Url;
		if (const FString* HrefPtr = RunInfo.MetaData.Find(TEXT("href")))
		{
			Url = *HrefPtr;
		}

		FName StyleName(*RunInfo.Name);
		const FTextBlockStyle TextStyle = Style->GetWidgetStyle<FTextBlockStyle>(StyleName);

		TSharedRef<SWidget> DecoratorWidget = SNew(SButton)
			.ContentPadding(FMargin(0))
			.ButtonStyle(FAppStyle::Get(), "NoBorder")
			.IsFocusable(true)
			.OnClicked_Lambda([Url]()
			{
				if (!Url.IsEmpty())
				{
					FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
				}
				return FReply::Handled();
			})
			[
				SNew(SRichTextBlock)
				.Text(RunInfo.Content)
				.TextStyle(&TextStyle)
				.DecoratorStyleSet(Style)
				.Visibility(EVisibility::Visible)
			];

		FTextRange ModelRange;
		ModelRange.BeginIndex = InOutModelText->Len();
		*InOutModelText += TEXT('\u200B');
		ModelRange.EndIndex = InOutModelText->Len();

		const FSlateFontInfo Font = TextStyle.Font;
		const float ShadowOffsetY = FMath::Min(0.0f, TextStyle.ShadowOffset.Y);

		TAttribute<int16> GetBaseline = TAttribute<int16>::CreateLambda([Font, ShadowOffsetY]()
		{
			const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			return static_cast<int16>(FontMeasure->GetBaseline(Font) - ShadowOffsetY);
		});

		FSlateWidgetRun::FWidgetRunInfo WidgetRunInfo(DecoratorWidget, GetBaseline);
		return FSlateWidgetRun::Create(TextLayout, RunInfo, InOutModelText, WidgetRunInfo, ModelRange);
	}

private:
	URichTextBlock* Owner;
	UEstLinkDecorator* Decorator;
};

UEstLinkDecorator::UEstLinkDecorator(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedPtr<ITextDecorator> UEstLinkDecorator::CreateDecorator(URichTextBlock* InOwner)
{
	return MakeShareable(new FEstLinkDecorator(InOwner, this));
}
