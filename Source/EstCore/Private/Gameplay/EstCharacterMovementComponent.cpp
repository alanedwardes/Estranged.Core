#include "Gameplay/EstCharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "EstCore.h"
#include "Gameplay/EstPlayer.h"
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
	MinJumpStepUpHeight = 10.f;
	MaxJumpStepUpHeight = 85.f;
	MaxJumpStepUpDistance = 34.f;
	JumpStepUpBoost = 10.f;
	JumpVelocityMultiplier = 1.25f;
	FootstepDistanceSpeedMultiplier = 0.4f;
	FootstepAngle = 64.f;
	FootstepTime = .25f;
	bCanSprint = true;

	FootstepIntensity = 1.f;
	FootstepIntensityCrouching = .5f;
	FootstepIntensityLand = 1.f;
	FootstepIntensityJump = 5.f;

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

bool UEstCharacterMovementComponent::ShouldFootstep() const
{
	// No manifest, no sounds
	if (FootstepManifest == nullptr)
	{
		return false;
	}

	// If we recently played a footstep, we shouldn't play one again
	if (LastFootstepTime > GetWorld()->GetTimeSeconds() - FootstepTime)
	{
		return false;
	}

	// If we've gone far enough, play
	const float Distance = (IsCrouching() ? MaxWalkSpeedCrouched : MaxWalkSpeed) * FootstepDistanceSpeedMultiplier;
	if (FVector::Dist(LastFootstepLocation, GetActorLocation()) > Distance)
	{
		return true;
	}

	// If we have turned far enough whilst moving, play
	if (!FVector::Coincident(LastFootstepDirection, GetPawnOwner()->GetActorForwardVector(), FMath::Cos(FootstepAngle)))
	{
		return true;
	}

	return false;
}

void UEstCharacterMovementComponent::DoFootstep(float Intensity)
{
	if (FootstepManifest == nullptr)
	{
		EST_LOG(this, Error, "UEstCharacterMovementComponent::DoFootstep() - Footstep manifest is null");
		return;
	}

	FCollisionQueryParams TraceParams(FName(TEXT("PlayerFootstepTrace")), true, GetOwner());
	TraceParams.bReturnPhysicalMaterial = true;

	const FVector EndTraceLocation = GetActorFeetLocation() + (FVector(0, 0, -1.f) * 100.f);

	FCollisionShape SweepCapsule = FCollisionShape::MakeCapsule(20.f, 0.f);

	FHitResult OutHit;
	GetWorld()->SweepSingleByProfile(OutHit, GetActorLocation(), EndTraceLocation, FQuat::Identity, PROFILE_FOOTSTEPS, SweepCapsule, TraceParams);

	const UPhysicalMaterial* PhysicalMaterial = FootstepMaterialOverride == nullptr ? UEstGameplayStatics::GetPhysicalMaterial(OutHit) : FootstepMaterialOverride;
	const FEstImpactEffect ImpactEffect = UEstGameplayStatics::FindImpactEffect(FootstepManifest, PhysicalMaterial);

	OnFootstep.Broadcast();

	for (USoundBase* ClothesSound : ClothesSounds)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ClothesSound, GetActorLocation());
	}

	if (ImpactEffect != FEstImpactEffect::None && OutHit.Component.IsValid())
	{
		UEstGameplayStatics::DeployImpactEffect(ImpactEffect, OutHit.Location, OutHit.Normal, OutHit.Component.Get(), Intensity, nullptr);
	}
	else if (OutHit.bBlockingHit && FootstepMaterialOverride == nullptr)
	{
		if (UEstGameplayStatics::IsDefaultPhysicalMaterial(PhysicalMaterial))
		{
			EST_LOG(this, Error, "Blocking hit on %s but no physical material", *UEstGameplayStatics::GetNameOrNull(OutHit.GetComponent()));
		}
		else
		{
			EST_LOG(this, Error, "Blocking hit on %s in actor %s but no impact effect in manifest %s", *UEstGameplayStatics::GetNameOrNull(PhysicalMaterial), *UEstGameplayStatics::GetNameOrNull(OutHit.GetComponent()), *UEstGameplayStatics::GetNameOrNull(FootstepManifest));
		}
	}

	LastFootstepLocation = GetActorLocation();
	LastFootstepDirection = GetPawnOwner()->GetActorForwardVector();
	LastFootstepTime = GetWorld()->GetTimeSeconds();
}

void UEstCharacterMovementComponent::MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult)
{
	Super::MoveAlongFloor(InVelocity, DeltaSeconds, OutStepDownResult);

	if (ShouldFootstep())
	{
		DoFootstep(IsCrouching() ? FootstepIntensityCrouching : FootstepIntensity);
	}
}

void UEstCharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	Super::ProcessLanded(Hit, remainingTime, Iterations);

	AEstPlayer* Player = Cast<AEstPlayer>(CharacterOwner);
	if (Player != nullptr)
	{
		Player->bForceCameraInterpolation = false;
	}

	DoFootstep(FootstepIntensityLand);
}

bool UEstCharacterMovementComponent::DoJump(bool bReplayingMoves, float DeltaTime)
{
	if (Super::DoJump(bReplayingMoves, DeltaTime))
	{
		Velocity = FVector(Velocity.X * JumpVelocityMultiplier, Velocity.Y * JumpVelocityMultiplier, Velocity.Z);
		DoFootstep(FootstepIntensityJump);
		return true;
	}

	return false;
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
	FVector PlayerForward = CharacterOwner->GetActorForwardVector();
	
	// Check if player is off the ladder and needs to unmount
	float DistanceFromStart = FVector::Dist(PlayerPosition, LadderExtents.StartPosition);
	float DistanceFromEnd = FVector::Dist(PlayerPosition, LadderExtents.EndPosition);
	float LadderLength = FVector::Dist(LadderExtents.StartPosition, LadderExtents.EndPosition);

	bool bMovingTowardsEnd = FVector::DotProduct(InputVector, LadderDirection) > 0.0f;
	bool bMovingTowardsStart = FVector::DotProduct(InputVector, -LadderDirection) > 0.0f;

	// If player is beyond the ladder extents, only unmount if they're moving further away
	if (DistanceFromStart > LadderLength && bMovingTowardsEnd)
	{
		DismountLadder(EEstLadderDismountReason::ReachedEnd);
		return;
	}

	if (DistanceFromEnd > LadderLength && bMovingTowardsStart)
	{
		DismountLadder(EEstLadderDismountReason::ReachedStart);
		return;
	}

	FFindFloorResult FloorResult;
	FindFloor(CharacterOwner->GetActorLocation(), FloorResult, false);
	if (FloorResult.IsWalkableFloor() && bMovingTowardsStart)
	{
		DismountLadder(EEstLadderDismountReason::ReachedFloor);
		return;
	}

	// Project the input vector onto the player's forward direction to get the intended movement
	float ForwardInput = FVector::DotProduct(InputVector, PlayerForward);
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
