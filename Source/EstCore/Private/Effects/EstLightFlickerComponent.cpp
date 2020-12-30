// Estranged is a trade mark of Alan Edwardes.

#include "Effects/EstLightFlickerComponent.h"
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

	if (Curve != nullptr && Light != nullptr)
	{
		float MinTime;
		float MaxTime;
		Curve->GetTimeRange(MinTime, MaxTime);

		float CurveLength = MaxTime - MinTime;
		float CurvePosition = MinTime + FMath::Fmod(GetWorld()->TimeSeconds, CurveLength);
		Light->SetIntensity(LightIntensity * Curve->GetFloatValue(CurvePosition));
	}
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

