// Estranged is a trade mark of Alan Edwardes.

#include "Rendering/EstLightFlickerComponent.h"
#include "Curves/CurveFloat.h"
#include "Components/LightComponent.h"

UEstLightFlickerComponent::UEstLightFlickerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UEstLightFlickerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Curve == nullptr || Light == nullptr)
	{
		return;
	}

	float MinTime;
	float MaxTime;
	Curve->GetTimeRange(MinTime, MaxTime);

	const float CurveLength = MaxTime - MinTime;
	const float CurvePosition = MinTime + FMath::Fmod(GetWorld()->TimeSeconds, CurveLength);
	const float CurveValue = Curve->GetFloatValue(CurvePosition);

	Light->SetIntensity(LightIntensity * CurveValue);
	OnFlicker.Broadcast(CurveValue);
}

void UEstLightFlickerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	if (GetOwner() != nullptr && Light == nullptr)
	{
		GetOwner()->ForEachComponent<ULightComponent>(false, [this](ULightComponent* InLightComponent) {
			if (InLightComponent->Mobility != EComponentMobility::Static && InLightComponent->bAffectsWorld)
			{
				Light = InLightComponent;
				LightIntensity = Light->Intensity;
			}
		});
	}
}

