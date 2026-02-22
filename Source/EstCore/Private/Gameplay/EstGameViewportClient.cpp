// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstGameViewportClient.h"
#include "EstCore.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstGameViewportClient)

TOptional<bool> UEstGameViewportClient::QueryShowFocus(const EFocusCause InFocusCause) const
{
	return FParse::Param(FCommandLine::Get(), TEXT("showfocus")) || bIsPlayInEditorViewport;
}