// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "UI/EstMenuBase.h"
#include "EstMenuModal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExit);

/**
 * 
 */
UCLASS()
class ESTCORE_API UEstMenuModal : public UEstMenuBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FOnExit OnExit;

	virtual void OnBack_Implementation() override;
};
