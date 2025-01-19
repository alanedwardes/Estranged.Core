// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EstPhysicsStrainComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESTCORE_API UEstPhysicsStrainComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEstPhysicsStrainComponent();

protected:
	UFUNCTION()
	virtual void OnRegister() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(float InDeltaTime, ELevelTick InTickType, FActorComponentTickFunction* InThisTickFunction) override;

private:
	class UFieldSystemComponent* FieldComponent;
	class UPrimitiveComponent* PrimitiveComponent;
};
