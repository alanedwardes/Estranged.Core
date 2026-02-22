#include "Gameplay/EstFootstepComponent.h"
#include "Gameplay/EstCharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "EstCore.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Physics/EstImpactManifest.h"
#include "Gameplay/EstGameInstance.h"
#include "Physics/EstImpactEffect.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstFootstepComponent)

UEstFootstepComponent::UEstFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.1f;

	FootstepDistanceSpeedMultiplier = 0.4f;
	FootstepAngle = 64.f;
	FootstepTime = 0.25f;
	FootstepIntensity = 1.f;
	FootstepIntensityCrouching = 0.5f;
	FootstepIntensityLand = 1.f;
	FootstepIntensityJump = 5.f;
}

void UEstFootstepComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	SetComponentTickEnabled(true);

	CharacterOwner = Cast<ACharacter>(GetOwner());
	if (CharacterOwner)
	{
		CharacterMovementComponent = Cast<UCharacterMovementComponent>(CharacterOwner->GetCharacterMovement());

		CharacterOwner->MovementModeChangedDelegate.AddUniqueDynamic(this, &UEstFootstepComponent::OnMovementModeChanged);
	}

	LastFootstepLocation = GetOwner()->GetActorLocation();
	LastFootstepDirection = GetOwner()->GetActorForwardVector();
	LastFootstepTime = GetWorld()->GetTimeSeconds();
}

void UEstFootstepComponent::Deactivate()
{
	Super::Deactivate();

	SetComponentTickEnabled(false);

	if (CharacterOwner)
	{
		CharacterOwner->MovementModeChangedDelegate.RemoveDynamic(this, &UEstFootstepComponent::OnMovementModeChanged);
	}

	CharacterMovementComponent = nullptr;
	CharacterOwner = nullptr;
}

void UEstFootstepComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	const bool bCouldFootstep = CharacterMovementComponent->IsMovingOnGround() || 
								CharacterMovementComponent->CustomMovementMode == (uint8)EEstCustomMovementMode::MOVE_Ladder ||
								UEstGameplayStatics::IsTreadingWater(CharacterMovementComponent);
	if (bCouldFootstep && ShouldFootstep())
	{
		float Intensity = CharacterMovementComponent->IsCrouching() ? FootstepIntensityCrouching : FootstepIntensity;
		DoFootstep(Intensity);
	}
}

bool UEstFootstepComponent::ShouldFootstep() const
{
	if (!CharacterMovementComponent)
	{
		return false;
	}

	if (FootstepManifest == nullptr)
	{
		return false;
	}

	if (LastFootstepTime > GetWorld()->GetTimeSeconds() - FootstepTime)
	{
		return false;
	}

	const float Distance = (CharacterMovementComponent->IsCrouching() ? CharacterMovementComponent->MaxWalkSpeedCrouched : CharacterMovementComponent->MaxWalkSpeed) * FootstepDistanceSpeedMultiplier;
	if (FVector::Dist(LastFootstepLocation, GetOwner()->GetActorLocation()) > Distance)
	{
		return true;
	}

	if (!FVector::Coincident(LastFootstepDirection, GetOwner()->GetActorForwardVector(), FMath::Cos(FootstepAngle)))
	{
		return true;
	}

	return false;
}

void UEstFootstepComponent::DoFootstep(float Intensity)
{
	if (FootstepManifest == nullptr)
	{
		EST_LOG(this, Error, "UEstFootstepComponent::DoFootstep() - Footstep manifest is null");
		return;
	}

	if (CharacterMovementComponent->CustomMovementMode == (uint8)EEstCustomMovementMode::MOVE_Ladder)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LadderSound, GetOwner()->GetActorLocation());
	}

	if (CharacterMovementComponent->IsMovingOnGround() || UEstGameplayStatics::IsTreadingWater(CharacterMovementComponent))
	{
		FCollisionQueryParams TraceParams(FName(TEXT("PlayerFootstepTrace")), true, GetOwner());
		TraceParams.bReturnPhysicalMaterial = true;

		const FVector EndTraceLocation = GetOwner()->GetActorLocation() + (FVector(0, 0, -1.f) * 100.f);

		FCollisionShape SweepCapsule = FCollisionShape::MakeCapsule(20.f, 0.f);

		FHitResult OutHit;
		GetWorld()->SweepSingleByProfile(OutHit, GetOwner()->GetActorLocation(), EndTraceLocation, FQuat::Identity, PROFILE_FOOTSTEPS, SweepCapsule, TraceParams);

		const UPhysicalMaterial* PhysicalMaterial = FootstepMaterialOverride == nullptr ? UEstGameplayStatics::GetPhysicalMaterial(OutHit) : FootstepMaterialOverride;
		const FEstImpactEffect ImpactEffect = UEstGameplayStatics::FindImpactEffect(FootstepManifest, PhysicalMaterial);

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
	}

	for (USoundBase* ClothesSound : ClothesSounds)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ClothesSound, GetOwner()->GetActorLocation());
	}

	OnFootstep.Broadcast();
	LastFootstepLocation = GetOwner()->GetActorLocation();
	LastFootstepDirection = GetOwner()->GetActorForwardVector();
	LastFootstepTime = GetWorld()->GetTimeSeconds();
}

void UEstFootstepComponent::OnMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	// Detect when character jumps (movement mode changes from Walking to Falling)
	if (PrevMovementMode == MOVE_Walking && CharacterMovementComponent && CharacterMovementComponent->MovementMode == MOVE_Falling)
	{
		// Additional check: ensure character has upward velocity (indicating a jump, not just falling off a ledge)
		FVector Velocity = CharacterMovementComponent->Velocity;
		if (FMath::IsNearlyEqual(Velocity.Z, CharacterMovementComponent->JumpZVelocity, 32.f))
		{
			DoFootstep(FootstepIntensityJump);
		}
	}

	// Detect when character lands (movement mode changes from non - Walking to Walking)
	if (PrevMovementMode != MOVE_Walking && CharacterMovementComponent && CharacterMovementComponent->MovementMode == MOVE_Walking)
	{
		DoFootstep(FootstepIntensityLand);
	}
}

