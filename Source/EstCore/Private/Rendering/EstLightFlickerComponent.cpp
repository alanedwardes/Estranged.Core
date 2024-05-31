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

	if (Lights.Num() == 0)
	{
		return;
	}

	float BrightnessAlpha = 1.f;
	if (Curve != nullptr)
	{

		float MinTime;
		float MaxTime;
		Curve->GetTimeRange(MinTime, MaxTime);

		const float CurveLength = MaxTime - MinTime;
		const float CurvePosition = MinTime + FMath::Fmod(GetWorld()->TimeSeconds + RandomOffset, CurveLength);
		BrightnessAlpha = Curve->GetFloatValue(CurvePosition);
	}
	else if (Style.Len() > 0)
	{
		int Frame = (int)(GetWorld()->TimeSeconds * 10);
		int WrappedFrame = Frame % Style.Len();
		int Brightness = (Style[WrappedFrame] - 'a');
		BrightnessAlpha = FMath::GetMappedRangeValueClamped(FVector2D(0, 25), FVector2D(0, 2), Brightness);
	}

	for (TPair<ULightComponent*, float> Light : Lights)
	{
		Light.Key->SetIntensity(FMath::FInterpTo(Light.Key->Intensity, Light.Value * BrightnessAlpha, DeltaTime, 100));
	}

	OnFlicker.Broadcast(BrightnessAlpha);
}

void UEstLightFlickerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Inject a random offset into the curve time, so not all flickers start at the same time
	RandomOffset = FMath::FRand() * 10.f;
	
	if (GetOwner() != nullptr && Lights.Num() == 0)
	{
		GetOwner()->ForEachComponent<ULightComponent>(false, [this](ULightComponent* InLightComponent) {
			if (InLightComponent->Mobility != EComponentMobility::Static && InLightComponent->bAffectsWorld)
			{
				Lights.Add(InLightComponent, InLightComponent->Intensity);
			}
		});
	}
}

