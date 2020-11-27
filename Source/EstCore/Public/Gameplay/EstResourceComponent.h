#pragma once

#include "Components/ActorComponent.h"
#include "Interfaces/EstSaveRestore.h"
#include "EstResourceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDepletedDelegate, AActor*, Instigator);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESTCORE_API UEstResourceComponent : public UActorComponent, public IEstSaveRestore
{
	GENERATED_BODY()

public:
	UEstResourceComponent();

// Begin IEstSaveRestore
	void OnPreRestore_Implementation() override {};
	void OnPostRestore_Implementation() override {};
	void OnPreSave_Implementation() override {};
	void OnPostSave_Implementation() override {};
// End IEstSaveRestore

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "Resource")
	virtual bool IsDepleted() { return FMath::IsNearlyZero(Resource); }

	UFUNCTION( BlueprintPure, Category = "Resource")
	virtual bool IsFull() { return FMath::IsNearlyEqual(Resource, MaxResource); }

	UFUNCTION(BlueprintPure, Category = "Resource")
	virtual float GetAlpha() { return Resource / MaxResource; }

	UFUNCTION(BlueprintPure, Category = "Resource")
	virtual float GetResource() { return Resource; }

	UFUNCTION(BlueprintPure, Category = "Resource")
	virtual float GetMaxResource() { return MaxResource; }

	UFUNCTION(BlueprintPure, Category = "Resource")
	virtual bool GetIsFrozen() { return IsFrozen; }

	UFUNCTION(BlueprintCallable, Category = "Resource")
	bool ChangeResource(float Amount, AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void SetIsFrozen(bool Frozen) { IsFrozen = Frozen; };

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void SetResource(float Amount) { Resource = Amount; };

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void SetMaxResource(float MaxAmount) { MaxResource = MaxAmount; };

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void SetChangePerSecond(float Amount) { ResourceChangePerSecond = Amount; };

	/** Called when the actor dies. */
	UPROPERTY(BlueprintAssignable, Category = "Resource")
	FDepletedDelegate OnDepleted;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Resource")
	bool IsFrozen;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Resource")
	float Resource;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Resource")
	float MaxResource;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Resource")
	float ResourceChangePerSecond;

	/** World time in seconds when this health component was first depleted. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Resource")
	float InitialDepletionTime;
};
