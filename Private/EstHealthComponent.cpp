#include "EstCore.h"
#include "EstHealthComponent.h"

void UEstHealthComponent::OnPostRestore_Implementation()
{
	Super::OnPostRestore_Implementation();

	if (IsDepleted())
	{
		GetOwner()->TakeDamage(0.f, FDamageEvent(), nullptr, nullptr);
		OnDeath.Broadcast(0.f, nullptr, nullptr, nullptr);
	}
}

void UEstHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UEstHealthComponent::TakeAnyDamage);
}

void UEstHealthComponent::EndPlay(const EEndPlayReason::Type Type)
{
	Super::EndPlay(Type);

	GetOwner()->OnTakeAnyDamage.RemoveAll(this);
}

void UEstHealthComponent::TakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Super::ChangeResource(-Damage, DamageCauser) && IsDepleted())
	{
		OnDeath.Broadcast(Damage, DamageType, InstigatedBy, DamageCauser);
	}
}