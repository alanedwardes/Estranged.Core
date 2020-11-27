#include "Gameplay/EstResourceComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "EstCore.h"

UEstResourceComponent::UEstResourceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = .1f;
	MaxResource = 100.f;
	Resource = 100.f;
}

void UEstResourceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner()->ActorHasTag(TAG_DEAD))
	{
		return;
	}

	if (FMath::IsNearlyZero(ResourceChangePerSecond))
	{
		return;
	}

	ChangeResource(ResourceChangePerSecond * DeltaTime, GetOwner());
}

bool UEstResourceComponent::ChangeResource(float Amount, AActor* Instigator)
{
	if (IsFrozen)
	{
		return false;
	}

	if (Amount > 0.f && IsFull())
	{
		return false;
	}

	if (Amount < 0.f && IsDepleted())
	{
		return false;
	}

	Resource = FMath::Clamp(Resource + Amount, 0.f, MaxResource);

	if (IsDepleted())
	{
		if (FMath::IsNearlyZero(InitialDepletionTime))
		{
			InitialDepletionTime = GetWorld()->GetTimeSeconds();
		}
		OnDepleted.Broadcast(Instigator);
	}

	return true;
}