#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/EstSaveRestore.h"
#include "EstCharacterMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFootstepDelegate);

UCLASS()
class UEstCharacterMovementComponent : public UCharacterMovementComponent, public IEstSaveRestore
{
	GENERATED_BODY()

public:
	UEstCharacterMovementComponent(const class FObjectInitializer& PCIP);

// Begin IEstSaveRestore
	void OnPreRestore_Implementation() override {};
	void OnPostRestore_Implementation() override;
	void OnPreSave_Implementation() override;
	void OnPostSave_Implementation() override {};
// End IEstSaveRestore

	virtual float GetMaxSpeed() const override;

	/** Is the character sprinting */
	UFUNCTION(BlueprintCallable, Category="Pawn|Components|CharacterMovement")
	virtual bool IsSprinting() const;

	/** The maximum ground speed when sprinting. */
	UPROPERTY(Category="Character Movement", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MaxSprintSpeed;

	UPROPERTY(Category = "Character Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MinJumpStepUpHeight;

	UPROPERTY(Category = "Character Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxJumpStepUpHeight;

	UPROPERTY(Category = "Character Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxJumpStepUpDistance;

	UPROPERTY(Category = "Character Movement", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float JumpStepUpBoost;

	UPROPERTY(Category = "Character Movement", SaveGame, EditAnywhere, BlueprintReadWrite)
	bool bCanSprint;

	UPROPERTY()
	bool bIsSprinting;

	UPROPERTY(Category = "Character Movement", EditAnywhere, BlueprintReadWrite)
	bool bIsMovementPaused;

	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite)
	TArray<class USoundBase*> ClothesSounds;

	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite)
	class UEstImpactManifest *FootstepManifest;

	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, SaveGame)
	class UPhysicalMaterial* FootstepMaterialOverride;

	UPROPERTY(BlueprintAssignable, Category = "Footsteps")
	FOnFootstepDelegate OnFootstep;

	/** Position of the last footstep sound */
	UPROPERTY()
	FVector LastFootstepLocation;

	/** Direction of the last footstep */
	UPROPERTY()
	FVector LastFootstepDirection;

	/** Time since last footstep */
	UPROPERTY()
	float LastFootstepTime;

	/** Distance the character has to travel to play a footstep. */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepDistanceSpeedMultiplier;

	/** Angle the character has to turn to play a footstep. */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepAngle;

	/** Minimum time in seconds between footsteps */
	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepTime;

	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepIntensity;

	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepIntensityCrouching;

	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepIntensityLand;

	UPROPERTY(Category = "Footsteps", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float FootstepIntensityJump;

	UPROPERTY(Category = "Character Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float JumpVelocityMultiplier;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	virtual void SetSprinting(bool IsSprinting);

	virtual bool ShouldFootstep() const;

	virtual void DoFootstep(float Intensity);

	virtual void MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult = NULL) override;

	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;

	virtual bool DoJump(bool bReplayingMoves) override;

	FORCEINLINE bool CanEverSprint() const { return bCanSprint; }

	UPROPERTY(SaveGame)
	float SAVE_MaxWalkSpeed;

	UPROPERTY(SaveGame)
	float SAVE_JumpZVelocity;

	UPROPERTY(SaveGame)
	bool SAVE_bCanJump;

	UPROPERTY(SaveGame)
	bool SAVE_bCanCrouch;

	UPROPERTY(SaveGame)
	bool SAVE_bIsCrouching;
};
