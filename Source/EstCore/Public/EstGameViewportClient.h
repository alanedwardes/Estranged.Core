// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "EstGameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API UEstGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
	
	virtual TOptional<bool> QueryShowFocus(const EFocusCause InFocusCause) const override;
};
