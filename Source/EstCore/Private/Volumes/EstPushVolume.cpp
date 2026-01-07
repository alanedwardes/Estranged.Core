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
	bGenerateOverlapEventsDuringLevelStreaming = true;
	Intensity = 1.f;
	PushBackMultiplier = 1.f;

	GetBrushComponent()->SetCollisionProfileName(PROFILE_TRIGGER);
}

void AEstPushVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (IsValid(Pawn))
	{
		Pawns.Add(Pawn);
	}

	UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent());
	if (Primitive != nullptr && Primitive->IsSimulatingPhysics())
	{
		Actors.Add(OtherActor);
	}
}

void AEstPushVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (IsValid(Pawn))
	{
		Pawns.Remove(Pawn);
	}

	Actors.Remove(OtherActor);
}

void AEstPushVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector AdditionalVelocity = (Push * Intensity) * DeltaTime;

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

		if (PushBackMultiplier < 1.0f && Intensity > 0.0f)
		{
			FVector NormalizedPush = Push;
			if (NormalizedPush.Normalize())
			{
				float VDotP = FVector::DotProduct(MovementComponent->Velocity, NormalizedPush);
				if (VDotP < 0.f)
				{
					FVector OpposingVelocity = NormalizedPush * VDotP;
					float ReductionFactor = (1.0f - PushBackMultiplier) * FMath::Min(Intensity, 1.0f);
					MovementComponent->Velocity -= OpposingVelocity * ReductionFactor;
				}
			}
		}

		MovementComponent->Velocity += AdditionalVelocity;
	}

	for (AActor* Actor : Actors)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
		if (!IsValid(Primitive))
		{
			continue;
		}

		Primitive->SetPhysicsLinearVelocity(AdditionalVelocity, true);
	}
}
