// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EstLevelWaypoint.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, abstract)
class ESTCORE_API UEstLevelWaypoint : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	FName Waypoint;
};
