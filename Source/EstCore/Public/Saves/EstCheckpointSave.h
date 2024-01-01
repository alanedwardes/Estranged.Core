// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Misc/DateTime.h"
#include "EstCheckpointSave.generated.h"

USTRUCT(BlueprintType)
struct FEstCheckpoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerStartTag;
};

/**
 * 
 */
UCLASS()
class ESTCORE_API UEstCheckpointSave : public USaveGame
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Checkpoints)
    TArray<FEstCheckpoint> Checkpoints;
};
