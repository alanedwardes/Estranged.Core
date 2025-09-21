#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/EstSaveRestore.h"
#include "EstCharacterMovementComponent.generated.h"

enum EEstLadderDismountReason : uint8;

/** Custom movement modes for EstCharacterMovementComponent */
UENUM(BlueprintType)
enum class EEstCustomMovementMode : uint8
{
	/** Default/None */
	MOVE_None		UMETA(DisplayName = "None"),
	
	/** Custom ladder climbing movement mode */
	MOVE_Ladder		UMETA(DisplayName = "Ladder"),
	
	MOVE_MAX		UMETA(Hidden)
};

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

	UPROPERTY(Category = "Character Movement", SaveGame, EditAnywhere, BlueprintReadWrite)
	bool bCanSprint;

	UPROPERTY()
	bool bIsSprinting;

	UPROPERTY(Category = "Character Movement", EditAnywhere, BlueprintReadWrite)
	bool bIsMovementPaused;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float LadderClimbSpeed;

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	virtual void SetSprinting(bool IsSprinting);

	/** Override PhysCustom to handle custom movement modes */
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	/** Get the current custom movement mode */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	EEstCustomMovementMode GetCustomMovementMode() const;

	/** Set a custom movement mode */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void SetCustomMovementMode(EEstCustomMovementMode NewCustomMode);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void MountLadder(TScriptInterface<class IEstLadder> NewLadder);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void DismountLadder(EEstLadderDismountReason DismountReason);

	/** Get the current ladder the character is climbing */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	TScriptInterface<class IEstLadder> GetCurrentLadder();

	/** Set the current ladder the character is climbing */
	UFUNCTION(BlueprintCallable, Category = "Pawn|Components|CharacterMovement")
	void SetCurrentLadder(TScriptInterface<class IEstLadder> NewLadder);

	UFUNCTION(BlueprintPure, Category = "Pawn|Components|CharacterMovement")
	bool IsClimbingLadder() const;

protected:
	/** Handle ladder climbing movement physics */
	virtual void PhysLadder(float deltaTime, int32 Iterations);

	/** Helper methods for PhysLadder */
	bool IsVerticalLadder(const FVector& LadderDirection) const;
	float CalculateVerticalInput(const FVector& InputVector, float InputMagnitude) const;
	float CalculateHorizontalInput(const FVector& InputVector, const FVector& LadderDirection, float InputMagnitude) const;
	float CalculateForwardInput(const FVector& InputVector, const FVector& LadderDirection) const;
	bool CheckLadderBounds(const FVector& PlayerPosition, const struct FLadderExtents& LadderExtents, float ForwardInput);
	bool CheckFloorDismount(float ForwardInput) const;

public:
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

private:
	/** The ladder the character is currently climbing, if any */
	TScriptInterface<class IEstLadder> CurrentLadder;
};
