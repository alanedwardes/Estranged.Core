// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstRotatingDoor.h"
#include "Components/ArrowComponent.h"
#include "Gameplay/EstBaseCharacter.h"
#include "Components/StaticMeshComponent.h"

AEstRotatingDoor::AEstRotatingDoor(const FObjectInitializer& ObjectInitializer)
{
	Scene = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Scene"));

	Arrow = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, TEXT("Arrow"));
	Arrow->SetupAttachment(Scene);

	DoorContainer = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("DoorContainer"));
	DoorContainer->SetupAttachment(Scene);

	DoorMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorContainer);

	DoorMaxAngle = 90.f;
	CurrentOpenDirection = EEstRotatingDoorDirection::Forwards;

	// By default, both directions are possible
	EnumAddFlags(PossibleOpenDirections, (uint8)EEstRotatingDoorDirection::Forwards);
	EnumAddFlags(PossibleOpenDirections, (uint8)EEstRotatingDoorDirection::Backwards);
}

void AEstRotatingDoor::UpdateDoor()
{
	EEstRotatingDoorDirection Direction = CurrentOpenDirection;

	// Check if the direction is allowed
	if (!EnumHasAnyFlags((EEstRotatingDoorDirection)PossibleOpenDirections, Direction))
	{
		// If not, use the opposite direction
		Direction = Direction == EEstRotatingDoorDirection::Forwards ? EEstRotatingDoorDirection::Backwards : EEstRotatingDoorDirection::Forwards;
	}

	float TargetZ = Direction == EEstRotatingDoorDirection::Forwards ? -DoorMaxAngle : DoorMaxAngle;
	float Rotation = UKismetMathLibrary::Ease(0.f, TargetZ, DoorOpenAmount, DoorEasingFunction);
	DoorContainer->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0.f, 0.f, Rotation)));
}

bool AEstRotatingDoor::TrySetDoorState(class AEstBaseCharacter* User, EEstDoorState NewDoorState)
{
	if (GetDoorState() == EEstDoorState::Closed && NewDoorState == EEstDoorState::Opening)
	{
		FVector DoorForward = GetActorForwardVector();
		FVector UserForward = User->GetActorForwardVector();
		CurrentOpenDirection = FVector::DotProduct(DoorForward, UserForward) < 0.f ? EEstRotatingDoorDirection::Backwards : EEstRotatingDoorDirection::Forwards;
	}

	return Super::TrySetDoorState(User, NewDoorState);
}

#if WITH_EDITOR
void AEstRotatingDoor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// If unset, set both as valid
	if ((EEstRotatingDoorDirection)PossibleOpenDirections == EEstRotatingDoorDirection::None)
	{
		EnumAddFlags(PossibleOpenDirections, (uint8)EEstRotatingDoorDirection::Forwards);
		EnumAddFlags(PossibleOpenDirections, (uint8)EEstRotatingDoorDirection::Backwards);
	}
}
#endif