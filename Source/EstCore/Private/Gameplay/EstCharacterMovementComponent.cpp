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
#include "Volumes/EstWaterVolume.h"

UEstCharacterMovementComponent::UEstCharacterMovementComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	MaxSprintSpeed = 600.0f;
	MaxWalkSpeed = 400.0f;
	MaxWalkSpeedCrouched = 200.0f;
	bCanSprint = true;

	LadderClimbSpeed = 128.f;
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
	
	// Store the initial relative position for moving ladder support
	InitialLadderRelativePosition = NearestPoint - LadderExtents.StartPosition;
	
	Velocity = FVector::ZeroVector;
}

void UEstCharacterMovementComponent::DismountLadder(EEstLadderDismountReason DismountReason)
{
	UObject* Ladder = CurrentLadder.GetObject();
	if (Ladder != nullptr)
	{
		IEstLadder::Execute_OnDismount(Ladder, CharacterOwner, DismountReason);
	}

	SetDefaultMovementMode();
	SetCurrentLadder(nullptr);
}

void UEstCharacterMovementComponent::PhysLadder(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	UObject* Ladder = CurrentLadder.GetObject();
	if (!Ladder || !CharacterOwner || (!CharacterOwner->GetController() && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!Ladder->Implements<UEstLadder>())
	{
		EST_LOG(this, Warning, "UEstCharacterMovementComponent::PhysLadder() - Current ladder does not implement IEstLadder, stopping ladder movement");
		SetDefaultMovementMode();
		SetCurrentLadder(nullptr);
		return;
	}

	FLadderExtents LadderExtents = IEstLadder::Execute_GetLadderExtents(Ladder);
	FVector LadderDirection = LadderExtents.GetLadderDirection();
	FVector InputVector = GetLastInputVector();
	FVector PlayerPosition = CharacterOwner->GetActorLocation();

	// Calculate input-based movement direction
	float ForwardInput = CalculateForwardInput(InputVector, LadderDirection);

	// Check for dismount conditions
	if (CheckLadderBounds(PlayerPosition, LadderExtents, ForwardInput))
	{
		return; // Already dismounted
	}

	if (IsVerticalLadder(LadderDirection) && CheckFloorDismount(ForwardInput))
	{
		DismountLadder(EEstLadderDismountReason::ReachedFloor);
		return;
	}

	// Calculate combined movement (ladder movement + input movement)
	FVector TotalMovement = FVector::ZeroVector;
	
	// Add ladder movement if ladder is movable
	AActor* LadderActor = Cast<AActor>(Ladder);
	if (LadderActor && LadderActor->GetRootComponent() && 
		LadderActor->GetRootComponent()->Mobility == EComponentMobility::Movable)
	{
		FVector TargetPosition = LadderExtents.StartPosition + InitialLadderRelativePosition;
		FVector LadderMovement = TargetPosition - PlayerPosition;
		
		// Only add ladder movement if it's significant to avoid jitter
		if (LadderMovement.Size() > 0.1f)
		{
			TotalMovement += LadderMovement;
		}
	}
	
	// Add input-based movement
	float ClimbSpeed = ForwardInput * LadderClimbSpeed;
	FVector InputMovement = LadderDirection * ClimbSpeed * deltaTime;
	TotalMovement += InputMovement;

	// Apply combined movement in a single operation
	if (TotalMovement.Size() > 0.0f)
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(TotalMovement, UpdatedComponent->GetComponentQuat(), true, Hit);
		
		// Update the relative position when player moves along the ladder
		if (ForwardInput != 0.0f)
		{
			FVector NewPlayerPosition = CharacterOwner->GetActorLocation();
			InitialLadderRelativePosition = NewPlayerPosition - LadderExtents.StartPosition;
		}
	}
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

float UEstCharacterMovementComponent::ImmersionDepth() const
{
	AEstWaterVolume* WaterVolume = Cast<AEstWaterVolume>(GetPhysicsVolume());
	if (WaterVolume == nullptr)
	{
		return Super::ImmersionDepth();
	}

	const FVector WaterSurface = WaterVolume->GetSurfaceAt(CharacterOwner->GetActorLocation());
	const float HalfHeight = CharacterOwner->GetSimpleCollisionHalfHeight();
	const float CharacterBottomZ = CharacterOwner->GetActorLocation().Z - HalfHeight;

	// Ratio of character height that is below water surface (0.0 = not in water, 1.0 = fully immersed)
	const float Depth = (WaterSurface.Z - CharacterBottomZ) / (HalfHeight * 2.f);
	return FMath::Clamp(Depth, 0.f, 1.f);
}

bool UEstCharacterMovementComponent::IsVerticalLadder(const FVector& LadderDirection) const
{
	float LadderZComponent = FMath::Abs(LadderDirection.Z);
	return LadderZComponent > 0.7f; // Threshold for considering ladder vertical
}

float UEstCharacterMovementComponent::CalculateVerticalInput(const FVector& InputVector, float InputMagnitude) const
{
	// For vertical ladders, use camera pitch-based movement
	FRotator ControlRotation = CharacterOwner->GetControlRotation();
	float PitchRadians = FMath::DegreesToRadians(ControlRotation.Pitch);
	
	// Get the forward component of input (positive for W, negative for S)
	FVector PlayerForward = CharacterOwner->GetActorForwardVector();
	float InputDirection = FVector::DotProduct(InputVector, PlayerForward);
	
	if (InputDirection > 0.0f) // W key - use camera direction
	{
		// +1 is straight up, -1 is straight down
		float PitchSine = FMath::Sin(PitchRadians) > 0 ? 1 : -1;
		return InputMagnitude * PitchSine;
	}
	else if (InputDirection < 0.0f) // S key - always move down
	{
		return -InputMagnitude; // Always move down
	}
	
	return 0.0f;
}

float UEstCharacterMovementComponent::CalculateHorizontalInput(const FVector& InputVector, const FVector& LadderDirection, float InputMagnitude) const
{
	// For horizontal ladders, use input direction relative to ladder direction
	float InputDirection = FVector::DotProduct(InputVector, LadderDirection);
	return InputMagnitude * FMath::Sign(InputDirection);
}

float UEstCharacterMovementComponent::CalculateForwardInput(const FVector& InputVector, const FVector& LadderDirection) const
{
	float InputMagnitude = InputVector.Size();
	
	if (InputMagnitude <= 0.0f)
	{
		return 0.0f;
	}
	
	if (IsVerticalLadder(LadderDirection))
	{
		return CalculateVerticalInput(InputVector, InputMagnitude);
	}
	else
	{
		return CalculateHorizontalInput(InputVector, LadderDirection, InputMagnitude);
	}
}

bool UEstCharacterMovementComponent::CheckLadderBounds(const FVector& PlayerPosition, const FLadderExtents& LadderExtents, float ForwardInput)
{
	// Check if player is off the ladder and needs to unmount
	float DistanceFromStart = FVector::Dist(PlayerPosition, LadderExtents.StartPosition);
	float DistanceFromEnd = FVector::Dist(PlayerPosition, LadderExtents.EndPosition);
	float LadderLength = FVector::Dist(LadderExtents.StartPosition, LadderExtents.EndPosition);

	// If player is beyond the ladder extents, only unmount if they're moving further away
	if (DistanceFromStart > LadderLength && ForwardInput > 0)
	{
		DismountLadder(EEstLadderDismountReason::ReachedEnd);
		return true; // Dismounted
	}

	if (DistanceFromEnd > LadderLength && ForwardInput < 0)
	{
		DismountLadder(EEstLadderDismountReason::ReachedStart);
		return true; // Dismounted
	}
	
	return false; // Still on ladder
}

bool UEstCharacterMovementComponent::CheckFloorDismount(float ForwardInput) const
{
	if (ForwardInput >= 0.0f)
	{
		return false; // Not moving down, don't check for floor
	}
	
	FFindFloorResult FloorResult;
	FindFloor(CharacterOwner->GetActorLocation(), FloorResult, false);
	return FloorResult.IsWalkableFloor();
}