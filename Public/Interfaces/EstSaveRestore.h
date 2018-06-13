#pragma once

#include "EstSaveRestore.generated.h"

/** Interface needed to cast both native classes and blueprints to the IUseable interface */
UINTERFACE(MinimalAPI)

class UEstSaveRestore : public UInterface
{
	GENERATED_BODY()
};

/** Interface added to every Actor that can be used */
class ESTCORE_API IEstSaveRestore
{
	GENERATED_IINTERFACE_BODY()

	/** Called before the object is saved. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPreSave();

	/** Called after the object is saved. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPostSave();

	/** Called before the object's state is restored. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPreRestore();

	/** Called after the object's state is restored. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnPostRestore();
};
