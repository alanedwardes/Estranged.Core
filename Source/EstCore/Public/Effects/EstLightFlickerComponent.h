// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EstLightFlickerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlickerSignature, float, CurveValue);

UCLASS(ClassGroup=(Custom), hidecategories = (Object, Collision, Activation, ActorComponent, Physics, Rendering, Mobility, LOD), meta=(BlueprintSpawnableComponent))
class ESTCORE_API UEstLightFlickerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEstLightFlickerComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCurveFloat* Curve;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitializeComponent() override;

	UPROPERTY(BlueprintAssignable, Category = Door)
	FFlickerSignature OnFlicker;

protected:
	UPROPERTY()
	class ULightComponent* Light;

	float LightIntensity;
};
