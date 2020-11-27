// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstGameViewportClient.h"
#include "EstCore.h"

TOptional<bool> UEstGameViewportClient::QueryShowFocus(const EFocusCause InFocusCause) const
{
	return FParse::Param(FCommandLine::Get(), TEXT("showfocus")) || EST_IN_VIEWPORT;
}