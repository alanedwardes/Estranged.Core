// Estranged is a trade mark of Alan Edwardes.

#include "Volumes/EstPushVolume.h"
#include "Components/BrushComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Pawn.h"

AEstPushVolume::AEstPushVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	GetBrushComponent()->SetCollisionProfileName(PROFILE_TRIGGER);
}

void AEstPushVolume::ActorEnteredVolume(AActor* Other)
{
	APawn* Pawn = Cast<APawn>(Other);
	if (IsValid(Pawn))
	{
		Pawns.Add(Pawn);
	}
}

void AEstPushVolume::ActorLeavingVolume(AActor* Other)
{
	APawn* Pawn = Cast<APawn>(Other);
	if (IsValid(Pawn))
	{
		Pawns.Remove(Pawn);
	}
}

void AEstPushVolume::Tick(float DeltaTime)
{
	for (const APawn* Pawn : Pawns)
	{
		if (!IsValid(Pawn))
		{
			continue;
		}

		UPawnMovementComponent* MovementComponent = Pawn->GetMovementComponent();
		if (!IsValid(MovementComponent))
		{
			continue;
		}

		MovementComponent->Velocity += Push * DeltaTime;
	}
}
