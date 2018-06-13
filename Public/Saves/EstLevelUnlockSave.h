// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EstPlayer.h"
#include "EstSave.h"
#include "EstLevelUnlockSave.generated.h"

UCLASS()
class ESTCORE_API UEstLevelUnlockSave : public UEstSave
{
	GENERATED_BODY()
	
public:
	UEstLevelUnlockSave()
	{
		SinglePlayerLevel = 0;
	}

	virtual FString GetSlotName()
	{
		return TEXT("LevelUnlocks");
	};

	UPROPERTY()
	int32 SinglePlayerLevel;

	UFUNCTION(BlueprintCallable)
	virtual bool SetLevelUnlocked(int32 LevelNumber)
	{
		if (LevelNumber <= SinglePlayerLevel)
		{
			return false;
		}

		SinglePlayerLevel = LevelNumber;
		return true;
	}

	UFUNCTION(BlueprintCallable)
	virtual bool IsLevelUnlocked(int32 LevelNumber)
	{
		return LevelNumber <= SinglePlayerLevel;
	}
};
