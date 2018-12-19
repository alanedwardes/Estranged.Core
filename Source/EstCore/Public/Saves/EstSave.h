// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EstSave.generated.h"

/**
 * 
 */
UCLASS(abstract)
class ESTCORE_API UEstSave : public USaveGame
{
	GENERATED_BODY()

public:
	virtual FString GetSlotName() PURE_VIRTUAL(UEstSave::GetSlotName, return FString(););
};
