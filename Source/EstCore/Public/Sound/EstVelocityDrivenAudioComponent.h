// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KismetAnimationTypes.h"
#include "EstVelocityDrivenAudioComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESTCORE_API UEstVelocityDrivenAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEstVelocityDrivenAudioComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	float LinearVelocityMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	float LinearVelocityMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	float AngularVelocityMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	float AngularVelocityMax;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = VelocityDrivenAudio)
	float LinearVelocityAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = VelocityDrivenAudio)
	float AngularVelocityAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	float VolumeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	float VolumeMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = VelocityDrivenAudio)
	int32 NumberOfSamples;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FPositionHistory PositionHistory;
	FPositionHistory RotationHistory;
};
