// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstBaseCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/EstRotatingDoor.h"

AEstRotatingDoor::AEstRotatingDoor(const FObjectInitializer& ObjectInitializer)
{
	Scene = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Scene"));

#if WITH_EDITOR
	Arrow = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, TEXT("Arrow"));
	Arrow->SetupAttachment(Scene);
#endif

	DoorContainer = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("DoorContainer"));
	DoorContainer->SetupAttachment(Scene);

	DoorMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorContainer);

	// By default, both directions are possible
	EnumAddFlags(PossibleOpenDirections, (uint8)EEstRotatingDoorDirection::Forwards);
	EnumAddFlags(PossibleOpenDirections, (uint8)EEstRotatingDoorDirection::Backwards);
}

void AEstRotatingDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Rotation = FMath::Lerp(0.f, CurrentOpenDirection == EEstRotatingDoorDirection::Forwards ? -90.f : 90.f, DoorOpenAmount);
	DoorContainer->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0.f, 0.f, Rotation)));
}

bool AEstRotatingDoor::TrySetDoorState(class AEstBaseCharacter* User, EEstDoorState NewDoorState)
{
	if (GetDoorState() == EEstDoorState::Closed && NewDoorState == EEstDoorState::Opening)
	{
		FVector DoorForward = GetActorForwardVector();
		FVector UserForward = User->GetActorForwardVector();

		EEstRotatingDoorDirection DesiredDirection = FVector::DotProduct(DoorForward, UserForward) < 0.f ? EEstRotatingDoorDirection::Backwards : EEstRotatingDoorDirection::Forwards;

		// Check if the direction is allowed
		if (!EnumHasAnyFlags((EEstRotatingDoorDirection)PossibleOpenDirections, DesiredDirection))
		{
			// If not, use the opposite direction
			DesiredDirection = DesiredDirection == EEstRotatingDoorDirection::Forwards ? EEstRotatingDoorDirection::Backwards : EEstRotatingDoorDirection::Forwards;
		}

		CurrentOpenDirection = DesiredDirection;
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