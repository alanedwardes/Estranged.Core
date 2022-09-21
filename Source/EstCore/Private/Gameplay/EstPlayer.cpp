#include "Gameplay/EstPlayer.h"
#include "EstCore.h"
#include "Gameplay/EstBaseWeapon.h"
#include "Gameplay/EstFirearmWeapon.h"
#include "Gameplay/EstPlayerController.h"
#include "Interfaces/EstCarryable.h"
#include "Interfaces/EstInteractive.h"
#include "Gameplay/EstCharacterMovementComponent.h"
#include "Gameplay/EstHealthComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SpotLightComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Runtime/Engine/Classes/PhysicsEngine/PhysicsHandleComponent.h"
#include "Gameplay/EstGameInstance.h"
#include "Gameplay/EstResourceComponent.h"
#include "DrawDebugHelpers.h"
#include "UI/EstHUDWidget.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogEstPlayer);
#define DOF_DISTANCE_MAX 10000.f

AEstPlayer::AEstPlayer(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Camera
	Camera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("Camera"));
	Camera->bUsePawnControlRotation = true;
	Camera->SetupAttachment(GetRootComponent());

	Camera->SetFieldOfView(110.f);
	CameraSmoothSpeed = 22.f;

	RestingFieldOfView = Camera->FieldOfView;
	ZoomFieldOfView = 40.f;

	// Interaction
	PlayerInteractionHeldUpdateSpeed = 50.f;
	PlayerInteractionDistance = 150.f;
	PlayerInteractionHeldDistance = 75.f;
	PlayerThrowLinearVelocity = 1000.f;
	PlayerMaximumCarryMass = 100.f;
	PlayerMaximumCarryRadius = 100.f;

	FlashlightPowerBurst = 1.f;
	FlashlightIntensity = 8.f;
	FlashlightOuterConeAngle = 26.f;
	FlashlightAttenuationRadius = 3000.f;

	Flashlight = PCIP.CreateDefaultSubobject<USpotLightComponent>(this, TEXT("Flashlight"));
	Flashlight->SetWorldTransform(GetActorTransform());
	Flashlight->SetInnerConeAngle(0.f);
	Flashlight->SetOuterConeAngle(FlashlightOuterConeAngle);
	Flashlight->SetAttenuationRadius(FlashlightAttenuationRadius);
	Flashlight->bUseInverseSquaredFalloff = false;
	Flashlight->LightFalloffExponent = 6.f;
	Flashlight->SetCastShadows(true);
	Flashlight->SetVisibility(false);
	Flashlight->SetIntensity(FlashlightIntensity);
	Flashlight->bUseTemperature = true;
	Flashlight->Temperature = 3750.f;
	Flashlight->ShadowResolutionScale = .5f; // Make it a little burry
	Flashlight->bAffectTranslucentLighting = false; // Stop it affecting glass, water

	Battery = PCIP.CreateDefaultSubobject<UEstResourceComponent>(this, TEXT("Battery"));
	BatteryConsumptionPerSecond = -.4f;

	Stamina = PCIP.CreateDefaultSubobject<UEstResourceComponent>(this, TEXT("Stamina"));
	Stamina->SetComponentTickInterval(1.f);
	StaminaGainPerSecond = 10.f;
	StaminaConsumptionPerSecond = -10.f;

	Oxygen = PCIP.CreateDefaultSubobject<UEstResourceComponent>(this, TEXT("Oxygen"));
	Oxygen->SetComponentTickInterval(1.f);

	UnderwaterTintColor = FLinearColor(.6f, .7f, 1.f);

	ViewModel = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("ViewModelScene"));
	ViewModel->AttachToComponent(Camera, FAttachmentTransformRules::KeepRelativeTransform);

	ViewModelMesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("ViewModelMesh"));
	ViewModelMesh->AttachToComponent(ViewModel, FAttachmentTransformRules::KeepRelativeTransform);
	ViewModelMesh->bSelfShadowOnly = true;
	ViewModelMesh->bCastInsetShadow = true;
	ViewModelMesh->bCastShadowAsTwoSided = true;

	CarryHandle = PCIP.CreateDefaultSubobject<UPhysicsHandleComponent>(this, TEXT("CarryHandle"));
	CarryHandle->LinearStiffness = 1500.f;
	CarryHandle->AngularStiffness = 3000.f;

	AimInterpolationSpeed = 18.f;
	AimingFieldOfView = 85.f;

	if (GEngine != nullptr)
	{
		GetCapsuleComponent()->SetMassOverrideInKg(NAME_None, 70.f, true);
		GetCapsuleComponent()->BodyInstance.bUseCCD = true;
		GetCapsuleComponent()->BodyInstance.SleepFamily = ESleepFamily::Sensitive;
	}

	GetCapsuleComponent()->SetCollisionProfileName(PROFILE_PLAYER);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedges = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->GravityScale = 2.5f;
	GetCharacterMovement()->Mass = 70.0f;
	GetCharacterMovement()->GroundFriction = 2.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 512.f;
	GetCharacterMovement()->bIgnoreBaseRotation = true;
	GetCharacterMovement()->PerchRadiusThreshold = 16.f;
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;
	GetCharacterMovement()->JumpZVelocity = 750.f;
	GetCharacterMovement()->Buoyancy = .99f;
	GetCharacterMovement()->AirControl = 0.25f;
	GetCharacterMovement()->AirControlBoostVelocityThreshold = .1f;
	GetCharacterMovement()->InitialPushForceFactor = 10.f;
	GetCharacterMovement()->PushForceFactor = 100000.f;
	GetCharacterMovement()->MaxTouchForce = 100.f;
	GetCharacterMovement()->RepulsionForce = 1.314f;
	GetCharacterMovement()->MaxSimulationTimeStep = 0.0166f;
	GetCharacterMovement()->StandingDownwardForceScale = 0.f;
	GetCharacterMovement()->MaxFlySpeed = 4096.f;
	GetCharacterMovement()->BrakingDecelerationFlying = 512.f;

	VelocityDamageThreshold = 700.f;
	VelocityDamageMinimum = 10.f;

	PlayerThrowAngularVelocity = FVector(0.f, 100.f, 500.f);
}

void AEstPlayer::OnPreSave_Implementation()
{
	Super::OnPreSave_Implementation();

	SAVE_bHasFlashlight = !Flashlight->bHiddenInGame;
	SAVE_bIsFlashlightOn = Flashlight->IsVisible();
	SAVE_bIsZooming = bZoomDesired;
	SAVE_HeldActorSaveId = HeldActor.IsValid() && HeldActor->Implements<UEstSaveRestore>() ? IEstSaveRestore::Execute_GetSaveId(HeldActor.Get()) : FGuid();
}

void AEstPlayer::OnPostRestore_Implementation()
{
	Super::OnPostRestore_Implementation();

	Flashlight->SetHiddenInGame(!SAVE_bHasFlashlight);

	if (SAVE_bIsFlashlightOn)
	{
		ToggleFlashlight();
	}

	if (SAVE_bIsZooming)
	{
		SetZooming(true);
	}

	AActor* LastHeldActor = UEstGameplayStatics::FindActorBySaveIdInWorld(this, SAVE_HeldActorSaveId);
	if (LastHeldActor)
	{
		PickUpActor(LastHeldActor);
	}
}

void AEstPlayer::PossessedBy(AController *NewController)
{
	Super::PossessedBy(NewController);

	APlayerController* PlayerController = Cast<APlayerController>(NewController);
	if (PlayerController)
	{
		PlayerCameraManager = PlayerController->PlayerCameraManager;

		if (!HUDWidgetClass.IsNull())
		{
			HUDWidget = CreateWidget<UEstHUDWidget>(PlayerController, HUDWidgetClass.LoadSynchronous());
			HUDWidget->AddToViewport(0);
		}
	}
	else
	{
		PlayerCameraManager = nullptr;
	}
}

void AEstPlayer::UnPossessed()
{
	Super::UnPossessed();
	PlayerCameraManager = nullptr;

	if (HUDWidget != nullptr)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}
}

float AEstPlayer::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	bool bWasDeadBefore = HealthComponent->IsDepleted();

	const float Result = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	// Passes if we were just killed, does not pass if damaged posthumous.
	if (!bWasDeadBefore && HealthComponent->IsDepleted())
	{
		UGameplayStatics::PlaySound2D(this, DeathSound);
	}

	if (HealthComponent->IsDepleted())
	{
		UnequipWeapon();
		BaseEyeHeight = 0.f;
		GetMovementComponent()->NavAgentProps.bCanCrouch = false;
		GetMovementComponent()->NavAgentProps.bCanJump = false;
		GetMovementComponent()->NavAgentProps.bCanSwim = false;
		GetMovementComponent()->NavAgentProps.bCanWalk = false;
		Controller->SetIgnoreMoveInput(true);
		Controller->SetIgnoreLookInput(true);
		GetCapsuleComponent()->SetCapsuleSize(16.f, 16.f);
	}

	if (Damage <= 0.f)
	{
		return Result;
	}

	if (IsHoldingActor() && !DamageEvent.DamageTypeClass.GetDefaultObject()->bCausedByWorld)
	{
		if (EventInstigator == nullptr || DamageCauser == nullptr)
		{
			DropHeldActor();
		}
		else
		{
			DropHeldActor(DamageCauser->GetActorForwardVector() * (Damage * 100.f));
		}
	}

	if (IsUsingObject())
	{
		IEstInteractive::Execute_OnUnused(UsingObject.Get(), this);
		UsingObject.Reset();
	}

	Stamina->SetResource(0.f);

	if (!bWasDeadBefore && !HealthComponent->GetIsFrozen())
	{
		USoundBase** Sound = DamageSounds.Find(DamageEvent.DamageTypeClass);
		if (Sound != nullptr)
		{
			UGameplayStatics::PlaySound2D(this, *Sound);
		}

		APlayerController* PlayerController = Cast<APlayerController>(Controller);
		const TSubclassOf<UCameraShakeBase>* Shake = DamageShakes.Find(DamageEvent.DamageTypeClass);
		if (PlayerController != nullptr && PlayerController->PlayerCameraManager != nullptr && Shake != nullptr)
		{
			PlayerController->PlayerCameraManager->StartCameraShake(*Shake);
		}

		UForceFeedbackEffect** ForceFeedback = DamageForceFeedback.Find(DamageEvent.DamageTypeClass);
		if (ForceFeedback != nullptr)
		{
			UEstGameplayStatics::ClientPlayForceFeedback(PlayerController, *ForceFeedback);
		}
	}
	
	return Result;
}

void AEstPlayer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SmoothZVelocity = FMath::FInterpConstantTo(SmoothZVelocity, FMath::Abs(GetVelocity().Z), DeltaSeconds, 1000.f);

	// Regenerate health when below 50%
	HealthComponent->SetChangePerSecond(HealthComponent->GetResource() < 50.f ? 1.f : 0.f);

	if (GetWorld()->TimeSince(InFrontCheckSubTickTime) > .25f)
	{
		InFrontCheckSubTick();
		InFrontCheckSubTickTime = GetWorld()->GetTimeSeconds();
	}

	if (GetWorld()->TimeSince(BlurFocusCheckSubTickTime) > .5f)
	{
		BlurFocusCheckSubTick();
		BlurFocusCheckSubTickTime = GetWorld()->GetTimeSeconds();
	}

	if (GetWorld()->TimeSince(InWaterCheckSubTickTime) > .1f)
	{
		InWaterCheckSubTick();
		InWaterCheckSubTickTime = GetWorld()->GetTimeSeconds();
	}

	UpdatePostProcessingTick(DeltaSeconds);
	UpdateCameraTick(DeltaSeconds);
	UpdateHeldActorTick(DeltaSeconds);
	UpdateFlashlightTick(DeltaSeconds);
	UpdateZoomTick(DeltaSeconds);
	UpdateViewModelTick(DeltaSeconds);

	MoveForwardInput(MoveForwardsAxis + MoveBackwardsAxis);
	MoveRightInput(MoveRightAxis + MoveLeftAxis);
	if (SwimmingUp)
	{
		AddMovementInput(FVector(0.f, 0.f, 1.f));
	}
}

void AEstPlayer::UpdatePostProcessingTick(float DeltaSeconds)
{
	if (bDisableVignette || !IsViewTarget())
	{
		Camera->PostProcessSettings.VignetteIntensity = 0.f;
	}
	else
	{
		const float VignetteIntensityTarget = (bIsAiming ? .75f : 0.5f) + (1.f - HealthComponent->GetAlpha());

		const float VignetteInterpolationSpeed = 4.f;
		Camera->PostProcessSettings.VignetteIntensity = FMath::FInterpTo(Camera->PostProcessSettings.VignetteIntensity, VignetteIntensityTarget, DeltaSeconds, VignetteInterpolationSpeed);
	}

	if (HeadInWater && IsViewTarget())
	{
		Camera->PostProcessSettings.bOverride_ScreenPercentage_DEPRECATED = true;
		Camera->PostProcessSettings.ScreenPercentage_DEPRECATED = 50.f;
	}
	else
	{
		Camera->PostProcessSettings.bOverride_ScreenPercentage_DEPRECATED = false;
		Camera->PostProcessSettings.ScreenPercentage_DEPRECATED = 100.f;
	}
}

void AEstPlayer::UpdateCameraTick(float DeltaSeconds)
{
	FVector Location;
	FRotator Rotator;
	GetActorEyesViewPoint(Location, Rotator);

	const float SmoothSpeed = GetCharacterMovement()->IsCrouching() || bForceCameraInterpolation ? CameraSmoothSpeed * .5 : GetCharacterMovement()->IsFalling() ? BIG_NUMBER : CameraSmoothSpeed;

	const FVector CurrentLocation = Camera->GetComponentLocation();
	if (bForceCameraInterpolation)
	{
		Camera->SetWorldLocationAndRotation(FMath::VInterpTo(CurrentLocation, Location, DeltaSeconds, SmoothSpeed), Rotator);
	}
	else
	{
		const float InteropZ = FMath::FInterpTo(CurrentLocation.Z, Location.Z, DeltaSeconds, SmoothSpeed);
		Camera->SetWorldLocationAndRotation(FVector(Location.X, Location.Y, InteropZ), Rotator);
	}
}

void AEstPlayer::BeginPlay()
{
	Super::BeginPlay();

	Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void AEstPlayer::UpdateHeldActorTick(float DeltaSeconds)
{
	if (!IsHoldingActor())
	{
		return;
	}

	if (HeldActor->GetClass()->ImplementsInterface(UEstCarryable::StaticClass()))
	{
		if (IEstCarryable::Execute_OnUpdatePosition(HeldActor.Get(), this, Camera->GetComponentTransform(), DeltaSeconds))
		{
			return;
		}
	}

	// Drop if the camera is no longer centered around the player
	if (!IsViewTarget())
	{
		UE_LOG(LogEstPlayer, Warning, TEXT("Dropping held actor %s because the player is no longer the camera target"), *HeldActor->GetName());
		DropHeldActor();
		return;
	}

	// Drop if the object is too far away
	const float HeldObjectDistance = GetDistanceTo(HeldActor.Get());
	if (HeldObjectDistance > 200.f)
	{
		UE_LOG(LogEstPlayer, Warning, TEXT("Dropping held actor %s because it is too far away (%.2fcm)"), *HeldActor->GetName(), HeldObjectDistance);
		DropHeldActor();
		return;
	}

	// Drop if standing on object
	const AActor* MovementBase = GetMovementBaseActor(this);
	if (MovementBase != nullptr && HeldActor == MovementBase)
	{
		UE_LOG(LogEstPlayer, Warning, TEXT("Dropping held actor %s because it is being stood on"), *HeldActor->GetName(), HeldObjectDistance);
		DropHeldActor();
		return;
	}

	FVector Origin;
	FVector BoxExtent;
	HeldActorBounds.GetCenterAndExtents(Origin, BoxExtent);

	const FVector HeldLocation = Camera->GetComponentLocation() + (Camera->GetForwardVector() * (BoxExtent.GetMax() + PlayerInteractionHeldDistance));
	const FVector DesiredLocation = HeldLocation - Origin - HeldPrimitiveTransform.GetLocation();
	const FRotator DesiredRotation = (GetCapsuleComponent()->GetComponentRotation() + HeldPrimitiveTransform.Rotator());

	CarryHandle->SetTargetLocationAndRotation(DesiredLocation, DesiredRotation);
	CarryHandle->GetGrabbedComponent()->WakeAllRigidBodies(); // Workaround for UE5 Chaos chaos
}

void AEstPlayer::UpdateFlashlightTick(float DeltaSeconds)
{
	if (!Flashlight->IsVisible())
	{
		Flashlight->SetWorldLocation(Camera->GetComponentLocation());

		// Make the flashlight point downwards
		FRotator DownRotation = Camera->GetComponentRotation();
		DownRotation.Pitch = -90.f;
		Flashlight->SetWorldRotation(DownRotation);
		Battery->SetChangePerSecond(0.f);
		return;
	}

	Battery->SetChangePerSecond(BatteryConsumptionPerSecond);

	if (FMath::IsNearlyZero(Battery->GetResource(), .1f))
	{
		Battery->ChangeResource(FlashlightPowerBurst, this);
		ToggleFlashlight();
	}

	FlashlightTime += DeltaSeconds;

	float FlickerAlpha;
	if (FlashlightFlicker == nullptr)
	{
		FlickerAlpha = Battery->GetAlpha();
	}
	else
	{
		float Min;
		float Max;

		FlashlightFlicker->GetTimeRange(Min, Max);

		const float Time = FMath::Fmod(FlashlightTime, Max);

		// Multiply the flicker intensity by the power level
		FlickerAlpha = FlashlightFlicker->GetFloatValue(Time) * Battery->GetAlpha();
	}

	// If power is lower, use the flicker, if not, just use the power
	const float FinalAlpha = FMath::Lerp(FlickerAlpha, Battery->GetAlpha(), Battery->GetAlpha());

	// Calculate the flashlight's offset (so it appears to be in the player's hand)
	const FVector Offset = IsHoldingActor() ? FVector(-25.f, -25.f, 0.f) : FVector(20.f, -15.f, -20.f);

	// Whether the player is close to a wall
	bool bIsCloseToWall = FocalHit.Distance < 128.f;

	// Set the flashlight cone larger if close to a wall
	float OuterConeTarget = bIsCloseToWall ? 60.f : FlashlightOuterConeAngle;
	float IntensityTarget = FlashlightIntensity * (bIsCloseToWall ? .5f : 1.f);
	float AttenuationTarget = FlashlightAttenuationRadius * (bIsCloseToWall ? .5f : 1.f);

	// Interpolate the flashlight cone change
	Flashlight->SetIntensity(FMath::Lerp(0.f, IntensityTarget, FinalAlpha));
	Flashlight->SetOuterConeAngle(FMath::FInterpTo(Flashlight->OuterConeAngle, OuterConeTarget, DeltaSeconds, 8.f));
	Flashlight->SetAttenuationRadius(FMath::FInterpTo(Flashlight->AttenuationRadius, AttenuationTarget, DeltaSeconds, 8.f));

	// Construct the target transform
	const FTransform Target = FTransform(Camera->GetRelativeRotation(), Camera->GetRelativeLocation() + Camera->GetRelativeRotation().RotateVector(Offset));
	
	// Interpolate, taking into account the player's speed
	Flashlight->SetRelativeTransform(UKismetMathLibrary::TInterpTo(Flashlight->GetRelativeTransform(), Target, DeltaSeconds, GetMovementComponent()->GetMaxSpeed() / 25.f));
}

void AEstPlayer::InFrontCheckSubTick()
{
	if (!HasWeapon())
	{
		SomethingInFront = false;
		return;
	}

	// AETODO: This needs re-writing, the output is fine
	// but the maths to get there is overly expensive
	FVector Location;
	FRotator Rotator;
	GetActorEyesViewPoint(Location, Rotator);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = true;

	const FTransform Transform = GetTransform();

	const FVector EyesLocal = Transform.InverseTransformPosition(Location) + FVector(0.f, 15.f, -15.f);

	const FVector WeaponBox(15.f, 15.f, 10.f);
	const FVector TestLocation = Transform.TransformPosition(EyesLocal) + (Rotator.Vector() * 40.f);

	if (GetWorld()->OverlapBlockingTestByChannel(TestLocation, Transform.Rotator().Quaternion(), CHANNEL_PLAYER_IN_FRONT, FCollisionShape::MakeBox(WeaponBox), Params))
	{
		SomethingInFront = true;
	}
	else
	{
		SomethingInFront = false;
	}
}

void AEstPlayer::InWaterCheckSubTick()
{
	FVector Location;
	FRotator Rotator;
	GetActorEyesViewPoint(Location, Rotator);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const bool bIsInWater = GetWorld()->OverlapBlockingTestByChannel(Location, Rotator.Quaternion(), CHANNEL_PLAYER_WATER_CHECK, FCollisionShape::MakeBox(FVector(1.f)), Params);
	if (bIsInWater && IsViewTarget())
	{
		Oxygen->SetChangePerSecond(-5.f);
		HeadInWater = true;

		if (Oxygen->IsDepleted() && GetWorld()->TimeSince(LastDrowningDamageTime) > 1.f)
		{
			FDamageEvent Event(DrowningDamageType);
			TakeDamage(10.f, Event, GetController(), this);
			LastDrowningDamageTime = GetWorld()->TimeSeconds;
		}
	}
	else
	{
		Oxygen->SetChangePerSecond(10.f);
		HeadInWater = false;
	}
}

void AEstPlayer::BlurFocusCheckSubTick()
{
	FVector Location;
	FRotator Rotator;
	GetActorEyesViewPoint(Location, Rotator);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (HeldActor.IsValid())
	{
		Params.AddIgnoredActor(HeldActor.Get());
	}
	Params.bReturnPhysicalMaterial = true;
	Params.bTraceComplex = true;

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, Location, Location + (Rotator.Vector() * DOF_DISTANCE_MAX), CHANNEL_PLAYER_BLUR_FOCUS, Params);

	const EPhysicalSurface PhysicalSurface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

	if (!Hit.bBlockingHit || PhysicalSurface == SURFACE_TYPE_GLASS || PhysicalSurface == SURFACE_TYPE_FOLIAGE || PhysicalSurface == SURFACE_TYPE_METAL_MESH)
	{
		FocalDistance = DOF_DISTANCE_MAX;
		return;
	}

	FocalHit = Hit;
	FocalDistance = FVector::Distance(GetActorLocation(), FocalHit.Location);
}

void AEstPlayer::UpdateZoomTick(float DeltaSeconds)
{
	const float Target = bIsAiming ? AimingFieldOfView : bZoomDesired ? ZoomFieldOfView : RestingFieldOfView;

	Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, Target, DeltaSeconds, 22.f);
}

void AEstPlayer::UpdateViewModelTick(float DeltaSeconds)
{
	const bool bIsViewModelVisible = IsViewTarget() && !HealthComponent->IsDepleted();

	// Only update if stale
	if (ViewModel->IsVisible() != bIsViewModelVisible)
	{
		ViewModel->SetVisibility(bIsViewModelVisible, true);
	}

	if (HasWeapon())
	{
		const bool ShouldHolster = SomethingInFront || IsHoldingActor() || HeadInWater;

		if (ShouldHolster)
		{
			if (!EquippedWeapon->IsHolstered())
			{
				EquippedWeapon->Holster();
			}
		}
		else if (!IsUsingObject())
		{
			if (EquippedWeapon->IsHolstered())
			{
				EquippedWeapon->Unholster();
			}
		}

		const FTransform TargetOffsetTransform = bIsAiming ? EquippedWeapon->AimingViewModelOffset : EquippedWeapon->NormalViewModelOffset;
		const FTransform NewOffsetTransform = UKismetMathLibrary::TInterpTo(ViewModelMesh->GetRelativeTransform(), TargetOffsetTransform, DeltaSeconds, AimInterpolationSpeed);

		ViewModelMesh->SetRelativeTransform(NewOffsetTransform);
	}
	else
	{
		ViewModelMesh->SetRelativeTransform(FTransform());
	}
}

void AEstPlayer::ToggleFlashlight()
{
	if (!IsViewTarget() || HealthComponent->IsDepleted() || Flashlight->bHiddenInGame)
	{
		return;
	}

	float Min;
	float Max;

	if (FlashlightFlicker != nullptr)
	{
		FlashlightFlicker->GetTimeRange(Min, Max);
		FlashlightTime = Min;
	}

	Flashlight->ToggleVisibility();
	UGameplayStatics::SpawnSoundAttached(FlashlightSound, Flashlight);
}

bool AEstPlayer::IsViewTarget()
{
	if (PlayerCameraManager == nullptr)
	{
		return false;
	}

	// If we are animating to a target, we might not be the target
	if (PlayerCameraManager->PendingViewTarget.Target != nullptr)
	{
		return false;
	}

	// If we're faded out, assume we're not the target
	if (UEstGameplayStatics::GetCameraFadeAmount(PlayerCameraManager) > 0.99f)
	{
		return false;
	}

	return PlayerCameraManager->ViewTarget.Target == this;
}

void AEstPlayer::ShowMenuSection(FName MenuSection)
{
	AEstPlayerController* EstPlayerController = Cast<AEstPlayerController>(Controller);
	EstPlayerController->SetPause(true);
	UEstGameplayStatics::GetEstGameInstance(this)->SetMenuVisibility(FEstMenuVisibilityContext(true, MenuSection));
	EstPlayerController->SetMenuFocusState(true);
}

bool AEstPlayer::ShouldShowLoadGameMenu()
{
	if (!HealthComponent->IsDepleted())
	{
		return false;
	}

	return GetWorld()->TimeSince(HealthComponent->InitialDepletionTime) > 2.5f;
}

void AEstPlayer::SetZooming(bool Zooming)
{
	bZoomDesired = Zooming;
}

void AEstPlayer::ToggleZoomInput(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	SetZooming(FMath::IsNegativeFloat(Value) == false);
}

void AEstPlayer::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	OnLandedDelegate.Broadcast(Hit, SmoothZVelocity);

	if (SmoothZVelocity > VelocityDamageThreshold)
	{
		const float Exceeded = SmoothZVelocity - VelocityDamageThreshold;
		const float Damage = Exceeded / 10.f;
		if (Damage > VelocityDamageMinimum)
		{
			UE_LOG(LogEstPlayer, Log, TEXT("Player's velocity exceeded %.2f by %.2f before hit, dealing %.2f damage"), VelocityDamageThreshold, Exceeded, Damage);
			UGameplayStatics::ApplyPointDamage(this, Damage, Hit.Normal, Hit, GetController(), Hit.GetActor(), FallDamageType);
			EstCharacterMovement->DoFootstep(5.f);
		}
	}
}

void AEstPlayer::InteractPressedInput()
{
	if (ShouldShowLoadGameMenu())
	{
		ShowLoadGameMenu();
		return;
	}

	if (HealthComponent->IsDepleted())
	{
		return;
	}

	if (IsHoldingActor())
	{
		DropHeldActor();
		return;
	}

	FHitResult Result;
	const float TraceRadii[] = {2.f, 16.f, 48.f};
	for (float Radius : TraceRadii)
	{
		if (DoInteractionTrace(Radius, Result))
		{
			return;
		}
	}

	// If we're using a controller, reload our primary weapon
	if (UEstGameplayStatics::IsUsingGamepad(this))
	{
		ReloadInput();
	}
}

void AEstPlayer::InteractReleasedInput()
{
	if (HealthComponent->IsDepleted())
	{
		return;
	}

	if (!IsUsingObject())
	{
		return;
	}

	IEstInteractive::Execute_OnUnused(UsingObject.Get(), this);
	UsingObject.Reset();
}

bool AEstPlayer::DoInteractionTrace(float TraceSphereRadius, FHitResult& Result)
{
	FVector CamLoc;
	FRotator CamRot;
	Controller->GetPlayerViewPoint(CamLoc, CamRot);

	// Calculate the alpha amount that the player is looking down - 0 is not looking down, 1 is looking fully down
	float LookingDownAmount = FMath::Abs(FMath::Clamp(CamRot.GetComponentForAxis(EAxis::Y), -90.f, 0.f)) / 90.f;

	// Subtract 0 - 16 units from the trace start height depending on the looking down alpha
	const FVector Start = CamLoc - FVector(0.f, 0.f, FMath::Lerp(0.f, 16.f, LookingDownAmount));
	const FVector ShootDir = CamRot.Vector();
	const FVector End = CamLoc + ShootDir * PlayerInteractionDistance;

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(FName(TEXT("PlayerInteractTrace")), true, this);
	TraceParams.bTraceComplex = true;

	FHitResult OutHit;
	GetWorld()->SweepSingleByChannel(OutHit, Start, End, CamRot.Quaternion(), CHANNEL_PLAYER_INTERACT, FCollisionShape::MakeSphere(TraceSphereRadius), TraceParams);

#ifdef EST_DEBUG_TRACES
	DrawDebugSphere(GetWorld(), OutHit.Location, TraceSphereRadius, 8, FColor::Red, false, 5.f);
#endif

	const FHitResult PreviousResult = Result;
	Result = OutHit;

	if (OutHit.GetActor() && OutHit.IsValidBlockingHit())
	{
		// If this is the same component, early out since nothing has changed
		if (PreviousResult.GetComponent() == OutHit.GetComponent())
		{
			return false;
		}

		if (OutHit.GetActor()->GetClass()->ImplementsInterface(UEstInteractive::StaticClass()))
		{
			UsingObject = OutHit.GetActor();
			if (IEstInteractive::Execute_OnUsed(UsingObject.Get(), this, OutHit.GetComponent()))
			{
				return true;
			}
		}

		// Execute on all interactive components inside the actor
		const TSet<UActorComponent*> InteractiveComponents = OutHit.GetActor()->GetComponents();
		for (UActorComponent* InteractiveComponent : InteractiveComponents)
		{
			if (InteractiveComponent->GetClass()->ImplementsInterface(UEstInteractive::StaticClass()))
			{
				UsingObject = InteractiveComponent;
				if (IEstInteractive::Execute_OnUsed(UsingObject.Get(), this, OutHit.GetComponent()))
				{
					return true;
				}
			}
		}

		// We're finished looking at the components, so
		// if we saw the actor before nothing will have changed
		if (PreviousResult.GetActor() == OutHit.GetActor())
		{
			return false;
		}

		const bool bCanHumanPickUp = UEstGameplayStatics::CanHumanPickUpActor(this, OutHit.GetActor(), PlayerMaximumCarryMass, PlayerMaximumCarryRadius);
		if (bCanHumanPickUp && IsViewTarget())
		{
			PickUpActor(OutHit.GetActor());
			return true;
		}
	}

	return false;
}

void AEstPlayer::PickUpActor(AActor* ActorToHold)
{
	ensure(ActorToHold);

	// Move the actor into the persistent level
	HeldActorBounds = ActorToHold->CalculateComponentsBoundingBoxInLocalSpace();
	HeldActor = UEstGameplayStatics::MoveActorToLevel(ActorToHold, GetLevel());

	HeldPrimitive = Cast<UPrimitiveComponent>(HeldActor->GetRootComponent());
	HeldPrimitive->SetEnableGravity(false);
	HeldPrimitive->SetAllUseCCD(true);

	UEstCarryableUserData* CarryableUserData = UEstGameplayStatics::GetCarryableUserDataFromMesh(HeldPrimitive.Get());
	HeldPrimitiveTransform = CarryableUserData == nullptr ? FTransform::Identity : CarryableUserData->CarryTransform;

	CarryHandle->GrabComponentAtLocationWithRotation(HeldPrimitive.Get(), NAME_None, HeldPrimitive->GetComponentLocation(), HeldPrimitive->GetComponentRotation());

	// Must be last, the act of picking up an actor may make it self destruct
	if (HeldActor->GetClass()->ImplementsInterface(UEstCarryable::StaticClass()))
	{
		IEstCarryable::Execute_OnPickedUp(HeldActor.Get(), this);
	}
}

void AEstPlayer::DropHeldActor(FVector LinearVelocity, FVector AngularVelocity)
{
	ensure(IsHoldingActor());

	CarryHandle->ReleaseComponent();

	GetCapsuleComponent()->IgnoreActorWhenMoving(HeldActor.Get(), false);

	HeldPrimitive->SetEnableGravity(true);
	HeldPrimitive->SetPhysicsLinearVelocity(GetRootComponent()->GetComponentVelocity() + LinearVelocity);
	HeldPrimitive->SetPhysicsAngularVelocityInDegrees(AngularVelocity);

	if (HeldActor->GetClass()->ImplementsInterface(UEstCarryable::StaticClass()))
	{
		IEstCarryable::Execute_OnPutDown(HeldActor.Get(), this);
	}

	OnDropHeldActor(HeldActor.Get());

	HeldPrimitive.Reset();
	HeldActor.Reset();
}

bool AEstPlayer::AddFlashlightPower(float Power)
{
	return Battery->ChangeResource(Power, this);
}

float AEstPlayer::GetFlashlightPower()
{
	return Battery->GetResource();
}

void AEstPlayer::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	InInputComponent->BindAxis("MoveForward", this, &AEstPlayer::MoveForwardInput);
	InInputComponent->BindAxis("MoveRight", this, &AEstPlayer::MoveRightInput);
	InInputComponent->BindAxis("Turn", this, &AEstPlayer::TurnInput);
	InInputComponent->BindAxis("LookUp", this, &AEstPlayer::LookUpInput);
	InInputComponent->BindAxis("Zoom", this, &AEstPlayer::ToggleZoomInput);

	InInputComponent->BindAction("Menu", IE_Released, this, &AEstPlayer::ShowMainMenu);
	InInputComponent->BindAction("Console", IE_Released, this, &AEstPlayer::ShowConsoleMenu);
	InInputComponent->BindAction("Interact", IE_Pressed, this, &AEstPlayer::InteractPressedInput);
	InInputComponent->BindAction("Interact", IE_Released, this, &AEstPlayer::InteractReleasedInput);
	InInputComponent->BindAction("Flashlight", IE_Pressed, this, &AEstPlayer::ToggleFlashlight);
	InInputComponent->BindAction("Crouch", IE_Pressed, this, &AEstPlayer::CrouchPressedInput);
	InInputComponent->BindAction("Crouch", IE_Released, this, &AEstPlayer::CrouchReleasedInput);
	InInputComponent->BindAction("Jump", IE_Pressed, this, &AEstPlayer::JumpPressedInput);
	InInputComponent->BindAction("Jump", IE_Released, this, &AEstPlayer::JumpReleasedInput);
	InInputComponent->BindAction("Reload", IE_Pressed, this, &AEstPlayer::ReloadInput);
	InInputComponent->BindAction("Sprint", IE_Pressed, this, &AEstPlayer::SprintPressedInput);
	InInputComponent->BindAction("Sprint", IE_Released, this, &AEstPlayer::SprintReleasedInput);
	InInputComponent->BindAction("PrimaryAttack", IE_Pressed, this, &AEstPlayer::PrimaryAttackPressedInput);
	InInputComponent->BindAction("PrimaryAttack", IE_Released, this, &AEstPlayer::PrimaryAttackReleasedInput);
	InInputComponent->BindAction("SecondaryAttack", IE_Pressed, this, &AEstPlayer::SecondaryAttackPressedInput);
	InInputComponent->BindAction("SecondaryAttack", IE_Released, this, &AEstPlayer::SecondaryAttackReleasedInput);

	InInputComponent->BindAction("MoveForward", IE_Pressed, this, &AEstPlayer::MoveForwardsPressedInput);
	InInputComponent->BindAction("MoveForward", IE_Released, this, &AEstPlayer::MoveForwardsReleasedInput);
	InInputComponent->BindAction("MoveBackward", IE_Pressed, this, &AEstPlayer::MoveBackwardsPressedInput);
	InInputComponent->BindAction("MoveBackward", IE_Released, this, &AEstPlayer::MoveBackwardsReleasedInput);

	InInputComponent->BindAction("MoveRight", IE_Pressed, this, &AEstPlayer::MoveRightPressedInput);
	InInputComponent->BindAction("MoveRight", IE_Released, this, &AEstPlayer::MoveRightReleasedInput);
	InInputComponent->BindAction("MoveLeft", IE_Pressed, this, &AEstPlayer::MoveLeftPressedInput);
	InInputComponent->BindAction("MoveLeft", IE_Released, this, &AEstPlayer::MoveLeftReleasedInput);
}

void AEstPlayer::MoveForwardInput(float Value)
{
	if (!IsViewTarget() || FMath::IsNearlyZero(Value))
	{
		return;
	}

	if (GetEstCharacterMovement()->IsSprinting() && GetVelocity().Size() > GetEstCharacterMovement()->MaxWalkSpeed)
	{
		Stamina->SetChangePerSecond(StaminaConsumptionPerSecond);
	}

	SetZooming(false);

	if (GetCharacterMovement()->IsSwimming() || GetCharacterMovement()->IsFlying())
	{
		AddMovementInput(Camera->GetForwardVector(), Value);
	}
	else
	{
		AddMovementInput(GetCapsuleComponent()->GetForwardVector(), Value);
	}
}

void AEstPlayer::MoveRightInput(float Value)
{
	if (!IsViewTarget() || FMath::IsNearlyZero(Value))
	{
		return;
	}

	SetZooming(false);

	// Get right vector
	const FVector Direction = FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, Value);
}

void AEstPlayer::TurnInput(float Value)
{
	AddControllerYawInput(Value);
}

void AEstPlayer::LookUpInput(float Value)
{
	AddControllerPitchInput(Value);
}

void AEstPlayer::CrouchPressedInput()
{
	if (ShouldShowLoadGameMenu())
	{
		ShowLoadGameMenu();
		return;
	}

	if (UEstGameplayStatics::IsUsingGamepad(this) && bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AEstPlayer::CrouchReleasedInput()
{
	if (UEstGameplayStatics::IsUsingGamepad(this))
	{
		return;
	}

	UnCrouch();
}

void AEstPlayer::JumpPressedInput()
{
	if (ShouldShowLoadGameMenu())
	{
		ShowLoadGameMenu();
		return;
	}

	SwimmingUp = true;
	Jump();
}

void AEstPlayer::JumpReleasedInput()
{
	SwimmingUp = false;
}

void AEstPlayer::FellOutOfWorld(const class UDamageType& dmgType)
{
	ShowLoadGameMenu();

	Super::FellOutOfWorld(dmgType);
}

bool AEstPlayer::CanSprint()
{
	return Super::CanSprint() && !Stamina->IsDepleted() && !bIsAiming;
}

void AEstPlayer::EquipWeapon_Implementation(AEstBaseWeapon* Weapon)
{
	Weapon = Cast<AEstBaseWeapon>(UEstGameplayStatics::MoveActorToLevel(Weapon, GetLevel()));

	Super::EquipWeapon_Implementation(Weapon);

	TArray<UPrimitiveComponent*> Primitives;
	Weapon->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		Primitive->bSelfShadowOnly = true;
		Primitive->MarkRenderStateDirty();
	}

	Weapon->AttachToComponent(ViewModelMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, Weapon->SocketName);
}

void AEstPlayer::UnequipWeapon_Implementation()
{
	if (!HasWeapon())
	{
		return;
	}

	TArray<UPrimitiveComponent*> Primitives;
	EquippedWeapon->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		Primitive->bSelfShadowOnly = false;
		Primitive->MarkRenderStateDirty();
	}

	Super::UnequipWeapon_Implementation();
}

void AEstPlayer::SprintPressedInput()
{
	if (UEstGameplayStatics::IsUsingGamepad(this) && GetEstCharacterMovement()->bIsSprinting)
	{
		GetEstCharacterMovement()->SetSprinting(false);
		Stamina->SetChangePerSecond(StaminaGainPerSecond);
	}
	else
	{
		GetEstCharacterMovement()->SetSprinting(true);
	}
}

void AEstPlayer::SprintReleasedInput()
{
	if (UEstGameplayStatics::IsUsingGamepad(this))
	{
		return;
	}

	GetEstCharacterMovement()->SetSprinting(false);
	Stamina->SetChangePerSecond(StaminaGainPerSecond);
}

void AEstPlayer::PlayPrimaryFire()
{
	if (HasWeapon() && ViewModelMesh->GetAnimInstance())
	{
		ViewModelMesh->GetAnimInstance()->Montage_Play(EquippedWeapon->PlayerAnimManifest.PrimaryAttack);

		if (EquippedWeapon->PrimaryAttackShake && IsViewTarget())
		{
			APlayerController* PlayerController = Cast<APlayerController>(GetController());
			if (PlayerController)
			{
				PlayerController->ClientStartCameraShake(EquippedWeapon->PrimaryAttackShake, bIsAiming ? 0.25f : 1.f);
			}
		}
	}
}

void AEstPlayer::PlaySecondaryFire()
{
	if (HasWeapon() && ViewModelMesh->GetAnimInstance())
	{
		ViewModelMesh->GetAnimInstance()->Montage_Play(EquippedWeapon->PlayerAnimManifest.SecondaryAttack);
	}
}

void AEstPlayer::PlayReload()
{
	if (HasWeapon() && ViewModelMesh->GetAnimInstance())
	{
		ViewModelMesh->GetAnimInstance()->Montage_Play(EquippedWeapon->PlayerAnimManifest.Reload);
	}
}

void AEstPlayer::ReloadInput()
{
	if (HasWeapon())
	{
		AEstFirearmWeapon* CurrentlyEquippedFirearm = Cast<AEstFirearmWeapon>(EquippedWeapon.Get());
		if (CurrentlyEquippedFirearm != nullptr)
		{
			CurrentlyEquippedFirearm->ReloadPressed();
		}
	}
}

void AEstPlayer::NotifyActorOnInputTouchBegin(const ETouchIndex::Type FingerIndex)
{
	Super::NotifyActorOnInputTouchBegin(FingerIndex);

	PrimaryAttackPressedInput();
}

void AEstPlayer::NotifyActorOnInputTouchEnd(const ETouchIndex::Type FingerIndex)
{
	Super::NotifyActorOnInputTouchEnd(FingerIndex);

	PrimaryAttackReleasedInput();
}

void AEstPlayer::PrimaryAttackPressedInput()
{
	if (ShouldShowLoadGameMenu())
	{
		ShowLoadGameMenu();
		return;
	}

	if (!IsViewTarget())
	{
		return;
	}

	// If the held actor is ICarryable, forward the event
	if (IsHoldingActor() && HeldActor->GetClass()->ImplementsInterface(UEstCarryable::StaticClass()))
	{
		if (IEstCarryable::Execute_OnPrimaryAttackPressed(HeldActor.Get(), this))
		{
			return;
		}
	}

	if (IsUsingObject())
	{
		return;
	}

	if (IsHoldingActor())
	{
		DropHeldActor(Camera->GetForwardVector() * PlayerThrowLinearVelocity, PlayerThrowAngularVelocity);
		return;
	}

	if (HasWeapon())
	{
		EquippedWeapon->PrimaryAttackStart();
	}
}

void AEstPlayer::PrimaryAttackReleasedInput()
{
	// If the held actor is ICarryable, forward the event
	if (IsHoldingActor() && HeldActor->GetClass()->ImplementsInterface(UEstCarryable::StaticClass()))
	{
		if (IEstCarryable::Execute_OnPrimaryAttackReleased(HeldActor.Get(), this))
		{
			return;
		}
	}

	if (HasWeapon())
	{
		EquippedWeapon->PrimaryAttackEnd();
	}
}

void AEstPlayer::SecondaryAttackPressedInput()
{
	if (!IsViewTarget())
	{
		return;
	}

	if (IsHoldingActor())
	{
		DropHeldActor();
		return;
	}

	if (HasWeapon())
	{
		if (EquippedWeapon->HasSecondaryAttack())
		{
			EquippedWeapon->SecondaryAttackStart();
		}
		else if (EquippedWeapon->bCanAim)
		{
			bIsAiming = true;
		}
	}
}

void AEstPlayer::SecondaryAttackReleasedInput()
{
	if (HasWeapon())
	{
		if (EquippedWeapon->HasSecondaryAttack())
		{
			EquippedWeapon->SecondaryAttackEnd();
		}
		else if (EquippedWeapon->bCanAim)
		{
			bIsAiming = false;
		}
	}
}
