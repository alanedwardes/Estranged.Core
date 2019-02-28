#include "EstCharacterMovementComponent.h"
#include "Runtime/Engine/Classes/PhysicalMaterials/PhysicalMaterial.h"
#include "EstCore.h"
#include "EstPlayer.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "EstBaseCharacter.h"
#include "EstImpactManifest.h"
#include "EstImpactEffect.h"

UEstCharacterMovementComponent::UEstCharacterMovementComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	MaxSprintSpeed = 600.0f;
	MaxWalkSpeed = 400.0f;
	MaxWalkSpeedCrouched = 200.0f;
	MinJumpStepUpHeight = 10.f;
	MaxJumpStepUpHeight = 85.f;
	MaxJumpStepUpDistance = 64.f;
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
		EST_WARNING(TEXT("UEstCharacterMovementComponent::DoFootstep() - Footstep manifest is null"));
		return;
	}

	FCollisionQueryParams TraceParams(FName(TEXT("PlayerFootstepTrace")), true, GetOwner());
	TraceParams.bReturnPhysicalMaterial = true;

	const FVector EndTraceLocation = GetActorFeetLocation() + (FVector(0, 0, -1.f) * 100.f);

	FCollisionShape SweepCapsule = FCollisionShape::MakeCapsule(20.f, 0.f);

	FHitResult OutHit;
	GetWorld()->SweepSingleByChannel(OutHit, GetActorLocation(), EndTraceLocation, FQuat::Identity, ECC_Visibility, SweepCapsule, TraceParams);

	const UPhysicalMaterial* PhysicalMaterial = FootstepMaterialOverride == nullptr ? OutHit.PhysMaterial.Get() : FootstepMaterialOverride;
	const FEstImpactEffect ImpactEffect = UEstGameplayStatics::FindImpactEffect(FootstepManifest, PhysicalMaterial);

	OnFootstep.Broadcast();

	for (USoundBase* ClothesSound : ClothesSounds)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ClothesSound, GetActorLocation());
	}

	if (ImpactEffect != FEstImpactEffect::None && OutHit.Component.IsValid())
	{
		UEstGameplayStatics::DeployImpactEffect(ImpactEffect, OutHit.Location, OutHit.Normal, OutHit.Component.Get(), Intensity, nullptr);
		UE_LOG(LogEstFootsteps, Log, TEXT("Playing for %s from hit on component %s"), *PhysicalMaterial->GetName(), *OutHit.Component->GetName());
	}
	else if (OutHit.bBlockingHit && FootstepMaterialOverride == nullptr)
	{
		if (PhysicalMaterial == nullptr)
		{
			UE_LOG(LogEstFootsteps, Error, TEXT("Blocking hit on %s but no physical material"), *OutHit.GetActor()->GetName());
		}
		else
		{
			UE_LOG(LogEstFootsteps, Error, TEXT("Blocking hit on %s but no impact effect"), *PhysicalMaterial->GetName());
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

bool UEstCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	if (Super::DoJump(bReplayingMoves))
	{
		Velocity = FVector(Velocity.X * JumpVelocityMultiplier, Velocity.Y * JumpVelocityMultiplier, Velocity.Z);
		DoFootstep(FootstepIntensityJump);
		bCanJumpUp = true;
		return true;
	}

	return false;
}

void UEstCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	if (!bCanJumpUp)
	{
		return Super::PhysFalling(deltaTime, Iterations);
	}

	if (Velocity.Z <= 0.f)
	{
		return Super::PhysFalling(deltaTime, Iterations);
	}

	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(PhysFalling), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);
	const FCollisionShape CapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_None);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

	const FVector End = UpdatedComponent->GetComponentLocation() + FVector(0.f, 0.f, MinJumpStepUpHeight);
	const FVector Start = UpdatedComponent->GetComponentLocation() + FVector(0.f, 0.f, MaxJumpStepUpHeight) + (UpdatedComponent->GetForwardVector() * MaxJumpStepUpDistance);

	FHitResult Result(1.f);
	GetWorld()->SweepSingleByChannel(Result, Start, End, FQuat::Identity, CollisionChannel, CapsuleShape, CapsuleParams, ResponseParam);
	if (!IsWalkable(Result))
	{
		return Super::PhysFalling(deltaTime, Iterations);
	}

	FVector FinalLocation = Result.Location + FVector(0.f, 0.f, JumpStepUpBoost);

	const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(FinalLocation, FQuat::Identity, CollisionChannel, CapsuleShape, CapsuleParams, ResponseParam);
	if (bEncroached)
	{
		EST_DEBUG(TEXT("Player tried to step up but was encroached, ignoring."));
		return Super::PhysFalling(deltaTime, Iterations);
	}

	FVector OriginalCameraLocation;

	bCanJumpUp = false;
	MoveUpdatedComponent(FinalLocation - UpdatedComponent->GetComponentLocation(), UpdatedComponent->GetComponentQuat(), false, nullptr, ETeleportType::TeleportPhysics);

	AEstPlayer* Player = Cast<AEstPlayer>(CharacterOwner);
	if (Player != nullptr)
	{
		Player->bForceCameraInterpolation = true;
	}

	EST_DEBUG(*FString::Printf(TEXT("Stepping up on to actor %s"), Result.Actor.IsValid() ? *Result.Actor->GetName() : TEXT("<none>")));
}
