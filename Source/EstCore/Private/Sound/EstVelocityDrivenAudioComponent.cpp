// Estranged is a trade mark of Alan Edwardes.

#include "Sound/EstVelocityDrivenAudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Components/AudioComponent.h"

UEstVelocityDrivenAudioComponent::UEstVelocityDrivenAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	LinearVelocityMin = 0.f;
	LinearVelocityMax = 256.f;

	AngularVelocityMin = 0.f;
	AngularVelocityMax = 10.f;

	VolumeMin = 0.f;
	VolumeMax = 1.f;

	PitchMin = 1.f;
	PitchMax = 2.f;

	NumberOfSamples = 16;
}

void UEstVelocityDrivenAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (SceneComponent != nullptr)
	{
		const FTransform& ComponentTransform = SceneComponent->GetComponentTransform();

		LinearVelocityAlpha = UKismetAnimationLibrary::K2_CalculateVelocityFromPositionHistory(DeltaTime, ComponentTransform.GetLocation(), PositionHistory, NumberOfSamples, LinearVelocityMin, LinearVelocityMax);
		AngularVelocityAlpha = UKismetAnimationLibrary::K2_CalculateVelocityFromPositionHistory(DeltaTime, ComponentTransform.GetRotation().Vector(), RotationHistory, NumberOfSamples, AngularVelocityMin, AngularVelocityMax);
	}

	if (AudioComponent != nullptr)
	{
		const float LargestVelocityAlpha = FMath::Max(LinearVelocityAlpha, AngularVelocityAlpha);

		AudioComponent->SetVolumeMultiplier(FMath::Lerp(VolumeMin, VolumeMax, LargestVelocityAlpha));
		AudioComponent->SetPitchMultiplier(FMath::Lerp(PitchMin, PitchMax, LargestVelocityAlpha));
	}
}

