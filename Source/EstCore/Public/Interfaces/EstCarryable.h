#pragma once

#include "Runtime/CoreUObject/Public/UObject/Interface.h"
#include "EstCarryable.generated.h"

UINTERFACE()
class ESTCORE_API UEstCarryable : public UInterface
{
	GENERATED_BODY()
};

/** Interface added to every Actor that can be carried by the player */
class ESTCORE_API IEstCarryable
{
	GENERATED_IINTERFACE_BODY()

	/** Called when the object is picked up by the player. */
	UFUNCTION(BlueprintNativeEvent)
	void OnPickedUp(class AEstBaseCharacter* Character);

	/** Called when the object is put down by the player. */
	UFUNCTION(BlueprintNativeEvent)
	void OnPutDown(class AEstBaseCharacter* Character);

	/** Called when the player hits primary fire. */
	UFUNCTION(BlueprintNativeEvent)
	bool OnPrimaryAttackPressed(ACharacter* Character);

	/** Called when the player releases primary fire. */
	UFUNCTION(BlueprintNativeEvent)
	bool OnPrimaryAttackReleased(ACharacter* Character);

	/** Called when the player releases primary fire. */
	UFUNCTION(BlueprintNativeEvent)
	bool OnUpdatePosition(ACharacter* Character, const FTransform &CameraTransform, float DeltaSeconds);
};
