#pragma once

#include "EstSaveRestore.generated.h"

UINTERFACE()
class ESTCORE_API UEstSaveRestore : public UInterface
{
	GENERATED_BODY()
};

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
