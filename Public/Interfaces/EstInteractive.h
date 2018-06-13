#pragma once

#include "EstInteractive.generated.h"

/** Interface needed to cast both native classes and blueprints to the IUseable interface */
UINTERFACE(MinimalAPI)

class UEstInteractive : public UInterface
{
	GENERATED_BODY()
};

/** Interface added to every Actor that can be used */
class ESTCORE_API IEstInteractive
{
	GENERATED_IINTERFACE_BODY()

	/** Called when the object is used by the user. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool OnUsed(class AEstBaseCharacter* User, class USceneComponent* UsedComponent);

	/** Called when the user stops using the object. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnUnused(class AEstBaseCharacter* User);
};
