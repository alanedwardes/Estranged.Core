// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EstGameMode.generated.h"

UCLASS()
class ESTCORE_API AEstGameMode : public AGameMode
{
	GENERATED_BODY()

	virtual void HandleMatchHasStarted() override;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = State)
	void OnPreBeginPlay();
};
