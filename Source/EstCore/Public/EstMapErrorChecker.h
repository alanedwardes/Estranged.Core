// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EstMapErrorChecker.generated.h"

UCLASS()
class ESTCORE_API AEstMapErrorChecker : public AActor
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
};
