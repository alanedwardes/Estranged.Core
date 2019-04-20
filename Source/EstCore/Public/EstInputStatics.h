#pragma once

#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "EstInputStatics.generated.h"

UENUM(BlueprintType)
enum class EEstControllerType : uint8
{
	Unknown,

	// Steam Controllers
	UnknownSteamController,
	SteamController,
	SteamControllerV2,

	// Other Controllers
	UnknownNonSteamController,
	XBox360Controller,
	XBoxOneController,
	PS3Controller,
	PS4Controller,
	WiiController,
	AppleController,
	AndroidController,
	SwitchProController,
	SwitchJoyConLeft,
	SwitchJoyConRight,
	SwitchJoyConPair,
	SwitchInputOnlyController,
	MobileTouch,
	GameCube,

	// Keyboards and Mice
	GenericKeyboard,
	GenericMouse,
};

UCLASS()
class ESTCORE_API UEstInputStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* Returns the types of all controllers connected to the system. */
	UFUNCTION(BlueprintPure, Category = Input)
	static TArray<EEstControllerType> GetConnectedControllerTypes();
};
