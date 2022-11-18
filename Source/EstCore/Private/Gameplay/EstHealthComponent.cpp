#include "Gameplay/EstHealthComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"
#include "EstCore.h"

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
	GetOwner()->OnTakeRadialDamage.AddDynamic(this, &UEstHealthComponent::TakeRadialDamage);
	GetOwner()->OnTakePointDamage.AddDynamic(this, &UEstHealthComponent::TakePointDamage);
}

void UEstHealthComponent::EndPlay(const EEndPlayReason::Type Type)
{
	GetOwner()->OnTakeAnyDamage.RemoveAll(this);
	GetOwner()->OnTakeRadialDamage.RemoveAll(this);
	GetOwner()->OnTakePointDamage.RemoveAll(this);

	Super::EndPlay(Type);
}

void UEstHealthComponent::TakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Super::ChangeResource(-Damage, DamageCauser) && IsDepleted())
	{
		OnDeath.Broadcast(Damage, DamageType, InstigatedBy, DamageCauser);
	}
}

void UEstHealthComponent::TakeRadialDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, FVector Origin, FHitResult HitInfo, AController * InstigatedBy, AActor * DamageCauser)
{
	LastDamageLocation = HitInfo.Location;
	LastDamageDirection = -HitInfo.Normal;
}

void UEstHealthComponent::TakePointDamage(AActor * DamagedActor, float Damage, AController * InstigatedBy, FVector HitLocation, UPrimitiveComponent * FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType * DamageType, AActor * DamageCauser)
{
	LastDamageLocation = HitLocation;
	LastDamageDirection = ShotFromDirection;
}
