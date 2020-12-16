// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstDoor.h"

AEstDoor::AEstDoor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEstDoor::BeginPlay()
{
	Super::BeginPlay();
}

void AEstDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DoorState == EEstDoorState::Opening)
	{
		DoorOpenAmount += DeltaTime;

		if (DoorOpenAmount > 1.f)
		{
			DoorOpenAmount = 1.f;
			SetDoorState(EEstDoorState::Opened);
		}
	}

	if (DoorState == EEstDoorState::Closing)
	{
		DoorOpenAmount -= DeltaTime;

		if (DoorOpenAmount < 0.f)
		{
			DoorOpenAmount = 0.f;
			SetDoorState(EEstDoorState::Closed);
		}
	}
}

void AEstDoor::SetDoorState(EEstDoorState NewDoorState)
{
	DoorState = NewDoorState;

	if (NewDoorState == EEstDoorState::Opening)
	{
		OnOpening.Broadcast(this);
	}

	if (NewDoorState == EEstDoorState::Opened)
	{
		OnOpened.Broadcast(this);
	}

	if (NewDoorState == EEstDoorState::Closing)
	{
		OnClosing.Broadcast(this);
	}

	if (NewDoorState == EEstDoorState::Closed)
	{
		OnClosed.Broadcast(this);
	}
}

bool AEstDoor::TrySetDoorState(class AEstBaseCharacter* User, EEstDoorState NewDoorState)
{
	if (IsLocked())
	{
		return false;
	}

	if ((GetDoorState() == EEstDoorState::Closed && NewDoorState == EEstDoorState::Opening) ||
		(GetDoorState() == EEstDoorState::Opened && NewDoorState == EEstDoorState::Closing))
	{
		SetDoorState(NewDoorState);
		return true;
	}

	return false;
}

#if WITH_EDITOR
void AEstDoor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (DoorState == EEstDoorState::Opened && FMath::IsNearlyZero(DoorOpenAmount))
	{
		DoorOpenAmount = 1.f;
	}

	if (DoorState == EEstDoorState::Closed)
	{
		DoorOpenAmount = 0.f;
	}
}
#endif

bool AEstDoor::OnUsed_Implementation(class AEstBaseCharacter* User, class USceneComponent* UsedComponent)
{
	if (GetDoorState() == EEstDoorState::Closed)
	{
		return TrySetDoorState(User, EEstDoorState::Opening);
	}

	if (GetDoorState() == EEstDoorState::Opened)
	{
		return TrySetDoorState(User, EEstDoorState::Closing);
	}

	return false;
}

void AEstDoor::OnPostRestore_Implementation()
{
}

void AEstDoor::OnPreSave_Implementation()
{
}

