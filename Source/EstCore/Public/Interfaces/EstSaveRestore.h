#pragma once

#include "EstSaveRestore.generated.h"

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EEstSaveFlags : uint8
{
	None = 0x00,
	IgnoreTransform = 0x01,
	Count UMETA(Hidden)
};
ENUM_CLASS_FLAGS(EEstSaveFlags)

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

	/** Called to get the storage flags for this actor/component. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	EEstSaveFlags GetSaveFlags();

	/** Called to get the customised name of an actor/component */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FGuid GetSaveId();
};
