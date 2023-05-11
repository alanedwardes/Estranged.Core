#pragma once

#include "Components/ActorComponent.h"
#include "EstResourceComponent.h"
#include "EstHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FDeathSignature, float, Damage, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESTCORE_API UEstHealthComponent : public UEstResourceComponent
{
	GENERATED_BODY()

public:
	void OnPostRestore_Implementation() override;

	UFUNCTION()
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type Type) override;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	virtual void TakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Damage")
	virtual void TakeRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Damage")
	virtual void TakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Damage")
	FVector LastDamageLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Damage")
	FVector LastDamageDirection;

	/** Called when the actor dies. */
	UPROPERTY(BlueprintAssignable, Category = "Damage")
	FDeathSignature OnDeath;
};
