// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "EstPlayerStart.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPlayerStartSignature, AEstPlayerStart*, PlayerStart, AController*, NewPlayer, APawn*, NewPawn);

/**
 * 
 */
UCLASS()
class ESTCORE_API AEstPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPlayerStartSignature OnPlayerStart;
};
