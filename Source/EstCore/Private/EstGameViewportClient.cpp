// Estranged is a trade mark of Alan Edwardes.

#include "EstCore.h"
#include "EstGameViewportClient.h"

TOptional<bool> UEstGameViewportClient::QueryShowFocus(const EFocusCause InFocusCause) const
{
	return FParse::Param(FCommandLine::Get(), TEXT("showfocus")) || EST_IN_VIEWPORT;
}