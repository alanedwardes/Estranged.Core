#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EstFootstepComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFootstepDelegate);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESTCORE_API UEstFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEstFootstepComponent();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	/** Clothes sounds to play with each footstep */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite)
	TArray<class USoundBase*> ClothesSounds;

	/** Ladder sounds to play with each ladder footstep */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite)
	class USoundBase* LadderSound;

	/** Manifest containing footstep impact effects */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite)
	class UEstImpactManifest* FootstepManifest;

	/** Override physical material for footsteps */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, SaveGame)
	class UPhysicalMaterial* FootstepMaterialOverride;

	/** Event broadcast when a footstep occurs */
	UPROPERTY(BlueprintAssignable, Category = "Footsteps")
	FOnFootstepDelegate OnFootstep;

	/** Distance the character has to travel to play a footstep */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepDistanceSpeedMultiplier;

	/** Angle the character has to turn to play a footstep */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepAngle;

	/** Minimum time in seconds between footsteps */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepTime;

	/** Normal footstep intensity */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepIntensity;

	/** Crouching footstep intensity */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepIntensityCrouching;

	/** Landing footstep intensity */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepIntensityLand;

	/** Jump footstep intensity */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepIntensityJump;

	/** Check if a footstep should be played */
	UFUNCTION(BlueprintCallable, Category = "Footsteps")
	virtual bool ShouldFootstep() const;

	/** Play a footstep with the given intensity */
	UFUNCTION(BlueprintCallable, Category = "Footsteps")
	virtual void DoFootstep(float Intensity);

	/** Handle movement mode changed event (for detecting jumps) */
	UFUNCTION()
	void OnMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);

private:
	/** Position of the last footstep sound */
	UPROPERTY()
	FVector LastFootstepLocation;

	/** Direction of the last footstep */
	UPROPERTY()
	FVector LastFootstepDirection;

	/** Time since last footstep */
	UPROPERTY()
	float LastFootstepTime;

	/** Reference to the character movement component */
	UPROPERTY()
	class UCharacterMovementComponent* CharacterMovementComponent;

	/** Reference to the character owner */
	UPROPERTY()
	class ACharacter* CharacterOwner;
};
