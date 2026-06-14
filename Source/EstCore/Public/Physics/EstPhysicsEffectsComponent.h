// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "EstPhysicsEffectsComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEstPhysicsEffectsComponent, Log, All);

USTRUCT(BlueprintType)
struct ESTCORE_API FPhyicsEffectsPhysicsVolumeInfo
{
	GENERATED_BODY()

	FPhyicsEffectsPhysicsVolumeInfo()
		: FPhyicsEffectsPhysicsVolumeInfo(nullptr, nullptr, nullptr)
	{
	}

	FPhyicsEffectsPhysicsVolumeInfo(class UPrimitiveComponent* InPrimitiveComponent,
		class APhysicsVolume* InPhysicsVolume,
		class AEstWaterVolume* InWaterVolume)
		: PrimitiveComponent(InPrimitiveComponent)
		, PhysicsVolume(InPhysicsVolume)
		, WaterVolume(InWaterVolume)
	{
	}

	bool operator==(const FPhyicsEffectsPhysicsVolumeInfo& Other) const { return PrimitiveComponent == Other.PrimitiveComponent && PhysicsVolume == Other.PhysicsVolume && WaterVolume == Other.WaterVolume; }
	bool operator!=(const FPhyicsEffectsPhysicsVolumeInfo& Other) const { return !(*this == Other); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UPrimitiveComponent* PrimitiveComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class APhysicsVolume* PhysicsVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AEstWaterVolume* WaterVolume;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESTCORE_API UEstPhysicsEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEstPhysicsEffectsComponent();

protected:
	UFUNCTION()
	virtual void OnRegister() override;

	UFUNCTION()
	virtual void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void OnChaosPhysicsCollision(const FChaosPhysicsCollisionInfo& CollisionInfo);

	UFUNCTION()
	virtual void OnChaosBreak(const FChaosBreakEvent& BreakEvent);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FPhyicsEffectsPhysicsVolumeInfo> ComponentPhysicsVolumes;

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

private:
	TMap<class UPrimitiveComponent*, class UEstPhysicsUserData*> ComponentUserData;

	virtual void ApplyBuoyancyForce(UPrimitiveComponent* PrimitiveComponent, APhysicsVolume* PhysicsVolume, AEstWaterVolume* WaterVolume, UEstPhysicsUserData* PhysicsUserData);
};
