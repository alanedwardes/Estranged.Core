// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/EstDoor.h"
#include "Kismet/KismetMathLibrary.h"
#include "EstRotatingDoor.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EEstRotatingDoorDirection : uint8
{
	None,
	Forwards,
	Backwards
};
ENUM_CLASS_FLAGS(EEstRotatingDoorDirection)

UCLASS(BlueprintType, meta = (Bitflags))
class ESTCORE_API AEstRotatingDoor : public AEstDoor
{
	GENERATED_BODY()

public:
	AEstRotatingDoor(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USceneComponent* DoorContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UArrowComponent* Arrow;

	void UpdateDoor() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Door)
	float DoorMaxAngle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Door)
	TEnumAsByte<EEasingFunc::Type> DoorEasingFunction;

	bool TrySetDoorState(class AEstBaseCharacter* User, EEstDoorState NewDoorState) override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (Bitmask, BitmaskEnum = EEstRotatingDoorDirection), Category = Door)
	uint8 PossibleOpenDirections;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Door)
	EEstRotatingDoorDirection CurrentOpenDirection;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
