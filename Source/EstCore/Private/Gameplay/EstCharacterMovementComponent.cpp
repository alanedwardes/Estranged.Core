#include "Gameplay/EstCharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "EstCore.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Gameplay/EstBaseCharacter.h"
#include "Physics/EstImpactManifest.h"
#include "Gameplay/EstGameInstance.h"
#include "Physics/EstImpactEffect.h"
#include "Interfaces/EstLadder.h"
#include "Kismet/GameplayStatics.h"

UEstCharacterMovementComponent::UEstCharacterMovementComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	MaxSprintSpeed = 600.0f;
	MaxWalkSpeed = 400.0f;
	MaxWalkSpeedCrouched = 200.0f;
	bCanSprint = true;

	LadderClimbSpeed = 200.f;
}

void UEstCharacterMovementComponent::OnPreSave_Implementation()
{
	SAVE_MaxWalkSpeed = MaxWalkSpeed;
	SAVE_JumpZVelocity = JumpZVelocity;
	SAVE_bCanJump = NavAgentProps.bCanJump;
	SAVE_bCanCrouch = NavAgentProps.bCanCrouch;
	SAVE_bIsCrouching = IsCrouching();
}

void UEstCharacterMovementComponent::OnPostRestore_Implementation()
{
	MaxWalkSpeed = SAVE_MaxWalkSpeed;
	JumpZVelocity = SAVE_JumpZVelocity;
	NavAgentProps.bCanCrouch = SAVE_bCanCrouch;
	NavAgentProps.bCanJump = SAVE_bCanJump;

	if (SAVE_bIsCrouching)
	{
		Crouch();
	}
}

float UEstCharacterMovementComponent::GetMaxSpeed() const
{
	if (bIsMovementPaused)
	{
		return 0.f;
	}

	if (IsSprinting())
	{
		return MaxSprintSpeed;
	}

	return Super::GetMaxSpeed();
}

bool UEstCharacterMovementComponent::IsSprinting() const
{
	AEstBaseCharacter* Character = Cast<AEstBaseCharacter>(CharacterOwner);
	if (Character != nullptr)
	{
		return bIsSprinting && IsWalking() && Character->CanSprint() && !IsCrouching();
	}

	return bIsSprinting && IsWalking();
}

void UEstCharacterMovementComponent::SetSprinting(bool IsSprinting)
{
	bIsSprinting = IsSprinting;
}

void UEstCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	switch (static_cast<EEstCustomMovementMode>(CustomMovementMode))
	{
	case EEstCustomMovementMode::MOVE_Ladder:
		PhysLadder(deltaTime, Iterations);
		break;
	case EEstCustomMovementMode::MOVE_None:
	default:
		Super::PhysCustom(deltaTime, Iterations);
		break;
	}
}

EEstCustomMovementMode UEstCharacterMovementComponent::GetCustomMovementMode() const
{
	return static_cast<EEstCustomMovementMode>(CustomMovementMode);
}

void UEstCharacterMovementComponent::SetCustomMovementMode(EEstCustomMovementMode NewCustomMode)
{
	SetMovementMode(MOVE_Custom, static_cast<uint8>(NewCustomMode));
}

void UEstCharacterMovementComponent::MountLadder(TScriptInterface<IEstLadder> NewLadder)
{
	if (NewLadder.GetObject() == nullptr)
	{
		return;
	}

	SetCurrentLadder(NewLadder);
	SetCustomMovementMode(EEstCustomMovementMode::MOVE_Ladder);

	// Find the nearest point on the ladder and snap to it
	FLadderExtents LadderExtents = IEstLadder::Execute_GetLadderExtents(NewLadder.GetObject());
	FVector LadderDirection = LadderExtents.GetLadderDirection();
	FVector PlayerPosition = CharacterOwner->GetActorLocation();
	FVector ToStart = PlayerPosition - LadderExtents.StartPosition;
	float Projection = FVector::DotProduct(ToStart, LadderDirection);
	FVector NearestPoint = LadderExtents.StartPosition + (LadderDirection * Projection);
	CharacterOwner->SetActorLocation(NearestPoint);
	Velocity = FVector::ZeroVector;
}

void UEstCharacterMovementComponent::DismountLadder(EEstLadderDismountReason DismountReason)
{
	UObject* Ladder = CurrentLadder.GetObject();
	if (Ladder != nullptr)
	{
		IEstLadder::Execute_OnDismount(Ladder, CharacterOwner, DismountReason);
	}

	SetMovementMode(MOVE_Walking);
	SetCurrentLadder(nullptr);
}

void UEstCharacterMovementComponent::PhysLadder(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->GetController() && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	UObject* Ladder = CurrentLadder.GetObject();
	if (Ladder == nullptr)
	{
		EST_LOG(this, Warning, "UEstCharacterMovementComponent::PhysLadder() - Current ladder does not implement IEstLadder, stopping ladder movement");
		SetMovementMode(MOVE_Walking);
		SetCurrentLadder(nullptr);
		return;
	}

	FLadderExtents LadderExtents = IEstLadder::Execute_GetLadderExtents(Ladder);

	FVector LadderDirection = LadderExtents.GetLadderDirection();

	FVector InputVector = GetLastInputVector();
	FVector PlayerPosition = CharacterOwner->GetActorLocation();


	FRotator ControlRotation = CharacterOwner->GetControlRotation();
	float PitchRadians = FMath::DegreesToRadians(ControlRotation.Pitch);
	float InputMagnitude = InputVector.Size();

	// Check if player is pressing S (downward input) - this should always move down
	// We need to check input relative to the player's forward direction, not world space
	FVector PlayerForward = CharacterOwner->GetActorForwardVector();
	float ForwardInputDot = FVector::DotProduct(InputVector, PlayerForward);
	bool bPressingDown = ForwardInputDot < 0.0f;

	float ForwardInput;
	if (bPressingDown)
	{
		// S key pressed - always move down regardless of camera direction
		ForwardInput = -InputMagnitude;
	}
	else
	{
		// W key or other input - use camera direction
		// +1 is straight up, -1 is straight down
		float PitchSine = FMath::Sin(PitchRadians) > -0.75 ? 1 : -1;
		ForwardInput = InputMagnitude * PitchSine;
	}
	
	// Check if player is off the ladder and needs to unmount
	float DistanceFromStart = FVector::Dist(PlayerPosition, LadderExtents.StartPosition);
	float DistanceFromEnd = FVector::Dist(PlayerPosition, LadderExtents.EndPosition);
	float LadderLength = FVector::Dist(LadderExtents.StartPosition, LadderExtents.EndPosition);

	// If player is beyond the ladder extents, only unmount if they're moving further away
	if (DistanceFromStart > LadderLength && ForwardInput > 0)
	{
		DismountLadder(EEstLadderDismountReason::ReachedEnd);
		return;
	}

	if (DistanceFromEnd > LadderLength && ForwardInput < 0)
	{
		DismountLadder(EEstLadderDismountReason::ReachedStart);
		return;
	}

	FFindFloorResult FloorResult;
	FindFloor(CharacterOwner->GetActorLocation(), FloorResult, false);
	if (FloorResult.IsWalkableFloor() && ForwardInput < 0)
	{
		DismountLadder(EEstLadderDismountReason::ReachedFloor);
		return;
	}

	float ClimbSpeed = ForwardInput * LadderClimbSpeed;

	FVector Delta = LadderDirection * ClimbSpeed * deltaTime;

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
}

TScriptInterface<IEstLadder> UEstCharacterMovementComponent::GetCurrentLadder()
{
	return CurrentLadder;
}

void UEstCharacterMovementComponent::SetCurrentLadder(TScriptInterface<IEstLadder> NewLadder)
{
	CurrentLadder = NewLadder;
}

bool UEstCharacterMovementComponent::IsClimbingLadder() const
{
	return GetCustomMovementMode() == EEstCustomMovementMode::MOVE_Ladder;
}
