// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstWebImageDecorator.h"

#include "Components/RichTextBlock.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Text/ITextDecorator.h"
#include "Misc/DefaultValueHelper.h"
#include "WebImage.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SCompoundWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EstWebImageDecorator)

class SEstInlineWebImage : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEstInlineWebImage)
			: _FadeDuration(0.25f)
			, _BackgroundTint(FLinearColor::White)
		{}
		SLATE_ARGUMENT(FString, Src)
		SLATE_ARGUMENT(float, FadeDuration)
		SLATE_ARGUMENT(const FSlateBrush*, BackgroundBrush)
		SLATE_ARGUMENT(FLinearColor, BackgroundTint)
	SLATE_END_ARGS()

	void Construct(
		const FArguments& InArgs,
		const FSlateBrush* ForegroundBrush,
		const FTextBlockStyle& TextStyle,
		TOptional<int32> Width,
		TOptional<int32> Height,
		EStretch::Type Stretch)
	{
		Src = InArgs._Src;
		FadeDuration = InArgs._FadeDuration;
		BackgroundTintCopy = InArgs._BackgroundTint;

		ForegroundBrushCopy = *ForegroundBrush;
		if (InArgs._BackgroundBrush)
		{
			BackgroundBrushCopy = *InArgs._BackgroundBrush;
		}

		WebImage = MakeShared<FWebImage>();
		WebImage->SetStandInBrush(&ForegroundBrushCopy);

		const TSharedRef<FSlateFontMeasure> FontMeasure =
			FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

		float IconHeight = (float)FontMeasure->GetMaxCharacterHeight(TextStyle.Font, 1.0f);
		if (ForegroundBrushCopy.ImageSize.Y > 0.f)
		{
			IconHeight = FMath::Min(IconHeight, ForegroundBrushCopy.ImageSize.Y);
		}
		if (Height.IsSet()) { IconHeight = static_cast<float>(Height.GetValue()); }

		float IconWidth = IconHeight;
		if (Width.IsSet()) { IconWidth = static_cast<float>(Width.GetValue()); }

		ChildSlot
		[
			SNew(SBox)
			.HeightOverride(IconHeight)
			.WidthOverride(IconWidth)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SScaleBox)
					.Stretch(Stretch)
					.StretchDirection(EStretchDirection::DownOnly)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(WebImage->Attr())
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(&BackgroundBrushCopy)
					.ColorAndOpacity(this, &SEstInlineWebImage::GetBackgroundBrushColor)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(&ForegroundBrushCopy)
					.ColorAndOpacity(this, &SEstInlineWebImage::GetForegroundBrushColor)
				]
			]
		];
	}

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override
	{
		if (!bDownloadStarted)
		{
			if (FSlateRect::DoRectanglesIntersect(MyCullingRect, AllottedGeometry.GetLayoutBoundingRect()))
			{
				bDownloadStarted = true;
				WebImage->BeginDownload(Src, FWebImage::FOnImageDownloaded::CreateSP(
					const_cast<SEstInlineWebImage*>(this), &SEstInlineWebImage::OnDownloadComplete));
			}
		}

		const double CurrentTime = FSlateApplication::Get().GetCurrentTime();
		if (FadeStartTime >= 0.0 && (CurrentTime - FadeStartTime) < (double)FadeDuration)
		{
			const_cast<SEstInlineWebImage*>(this)->Invalidate(EInvalidateWidgetReason::Paint);
		}

		return SCompoundWidget::OnPaint(
			Args, AllottedGeometry, MyCullingRect,
			OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}

private:
	void OnDownloadComplete(bool bSuccess)
	{
		if (bSuccess)
		{
			FadeStartTime = FSlateApplication::Get().GetCurrentTime();
			Invalidate(EInvalidateWidgetReason::Paint);
		}
	}

	FSlateColor GetBackgroundBrushColor() const
	{
		return GetPlaceholderColor(BackgroundTintCopy);
	}

	FSlateColor GetForegroundBrushColor() const
	{
		return GetPlaceholderColor(FLinearColor::White);
	}

	FLinearColor GetPlaceholderColor(const FLinearColor& Tint) const
	{
		if (FadeStartTime < 0.0)
		{
			return Tint;
		}
		const double Elapsed = FSlateApplication::Get().GetCurrentTime() - FadeStartTime;
		const float Alpha = 1.f - FMath::Clamp((float)Elapsed / FadeDuration, 0.f, 1.f);
		return FLinearColor(Tint.R, Tint.G, Tint.B, Tint.A * Alpha);
	}

	FSlateBrush ForegroundBrushCopy;
	FSlateBrush BackgroundBrushCopy;
	FLinearColor BackgroundTintCopy;
	FString Src;
	TSharedPtr<FWebImage> WebImage;
	float FadeDuration = 0.25f;

	mutable bool bDownloadStarted = false;
	mutable double FadeStartTime = -1.0;
};

class FEstInlineWebImage : public FRichTextDecorator
{
public:
	FEstInlineWebImage(URichTextBlock* InOwner, UEstWebImageDecorator* InDecorator)
		: FRichTextDecorator(InOwner)
		, Decorator(InDecorator)
	{
	}

	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override
	{
		return RunParseResult.Name == TEXT("img")
			&& RunParseResult.MetaData.Contains(TEXT("src"));
	}

protected:
	virtual TSharedPtr<SWidget> CreateDecoratorWidget(
		const FTextRunInfo& RunInfo,
		const FTextBlockStyle& TextStyle) const override
	{
		const FSlateBrush* ForegroundBrush = &Decorator->ForegroundPlaceholderBrush;
		const FString Src = RunInfo.MetaData[TEXT("src")];

		FLinearColor BackgroundTint = FLinearColor::White;
		if (const FString* BackgroundString = RunInfo.MetaData.Find(TEXT("background")))
		{
			BackgroundTint = FColor::FromHex(*BackgroundString).ReinterpretAsLinear();
		}

		TOptional<int32> Width;
		if (const FString* WidthString = RunInfo.MetaData.Find(TEXT("width")))
		{
			int32 WidthTemp;
			if (FDefaultValueHelper::ParseInt(*WidthString, WidthTemp))
			{
				Width = WidthTemp;
			}
		}

		TOptional<int32> Height;
		if (const FString* HeightString = RunInfo.MetaData.Find(TEXT("height")))
		{
			int32 HeightTemp;
			if (FDefaultValueHelper::ParseInt(*HeightString, HeightTemp))
			{
				Height = HeightTemp;
			}
		}

		EStretch::Type Stretch = EStretch::ScaleToFit;
		if (const FString* StretchString = RunInfo.MetaData.Find(TEXT("stretch")))
		{
			const UEnum* StretchEnum = StaticEnum<EStretch::Type>();
			const int64 StretchValue = StretchEnum->GetValueByNameString(*StretchString);
			if (StretchValue != INDEX_NONE)
			{
				Stretch = static_cast<EStretch::Type>(StretchValue);
			}
		}

		return SNew(SEstInlineWebImage, ForegroundBrush, TextStyle, Width, Height, Stretch)
			.Src(Src)
			.FadeDuration(Decorator->FadeDuration)
			.BackgroundBrush(&Decorator->BackgroundPlaceholderBrush)
			.BackgroundTint(BackgroundTint);
	}

private:
	UEstWebImageDecorator* Decorator;
};

UEstWebImageDecorator::UEstWebImageDecorator(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedPtr<ITextDecorator> UEstWebImageDecorator::CreateDecorator(URichTextBlock* InOwner)
{
	return MakeShareable(new FEstInlineWebImage(InOwner, this));
}
