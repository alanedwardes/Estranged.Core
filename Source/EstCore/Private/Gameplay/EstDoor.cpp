// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstDoor.h"

AEstDoor::AEstDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	SetActorTickEnabled(false);

	DoorSpeed = 1.f;
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
		DoorOpenAmount += DeltaTime * DoorSpeed;

		if (DoorOpenAmount > 1.f)
		{
			DoorOpenAmount = 1.f;
			SetDoorState(EEstDoorState::Opened);
		}

		UpdateDoor();
	}

	if (DoorState == EEstDoorState::Closing)
	{
		DoorOpenAmount -= DeltaTime * DoorSpeed;

		if (DoorOpenAmount < 0.f)
		{
			DoorOpenAmount = 0.f;
			SetDoorState(EEstDoorState::Closed);
		}

		UpdateDoor();
	}
}

void AEstDoor::UpdateDoor()
{
}

void AEstDoor::SetDoorState(EEstDoorState NewDoorState)
{
	DoorState = NewDoorState;

	if (NewDoorState == EEstDoorState::Opening)
	{
		Opening();
		OnOpening.Broadcast(this);
	}

	if (NewDoorState == EEstDoorState::Opened)
	{
		Opened();
		OnOpened.Broadcast(this);
	}

	if (NewDoorState == EEstDoorState::Closing)
	{
		Closing();
		OnClosing.Broadcast(this);
	}

	if (NewDoorState == EEstDoorState::Closed)
	{
		Closed();
		OnClosed.Broadcast(this);
	}
}

bool AEstDoor::TrySetDoorState(class AEstBaseCharacter* User, EEstDoorState NewDoorState)
{
	if (IsLocked())
	{
		if (NewDoorState == EEstDoorState::Opening)
		{
			OpenFailed();
			OnOpenFailed.Broadcast(this);
		}
		return false;
	}

	if ((GetDoorState() == EEstDoorState::Closed && NewDoorState == EEstDoorState::Opening) ||
		(GetDoorState() == EEstDoorState::Opened && NewDoorState == EEstDoorState::Closing))
	{
		SetDoorState(NewDoorState);
		return true;
	}

	SetActorTickEnabled(true);

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

	if (DoorState == EEstDoorState::Closed && FMath::IsNearlyZero(DoorOpenAmount))
	{
		DoorOpenAmount = 0.f;
	}

	UpdateDoor();
}
#endif

bool AEstDoor::OnUsed_Implementation(class AEstBaseCharacter* User, class USceneComponent* UsedComponent)
{
	if (GetDoorState() == EEstDoorState::Closed || GetDoorState() == EEstDoorState::Opening)
	{
		return TrySetDoorState(User, EEstDoorState::Opening);
	}

	if (GetDoorState() == EEstDoorState::Opened || GetDoorState() == EEstDoorState::Closing)
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

