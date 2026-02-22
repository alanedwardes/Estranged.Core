// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstMenuSection.h"
#include "UI/EstUIStatics.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstMenuSection)

ESlateVisibility UEstMenuSection::VisibleOnDesktop() const
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	return ESlateVisibility::SelfHitTestInvisible;
#else
	return ESlateVisibility::Collapsed;
#endif
}
