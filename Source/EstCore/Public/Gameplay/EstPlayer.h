#pragma once

#include "Components/PrimitiveComponent.h"
#include "EstBaseCharacter.h"
#include "EstPlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnShowHintDelegate, TArray<FName>, Bindings, FText, Label, float, ShowTime, FVector, WorldLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHideHintDelegate);

/** The base class for all players in Estranged. */
UCLASS(abstract)
class ESTCORE_API AEstPlayer : public AEstBaseCharacter
{
	GENERATED_BODY()

public:
	AEstPlayer(const class FObjectInitializer& PCIP);

	void OnPreSave_Implementation() override;
	void OnPostRestore_Implementation() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	class APlayerCameraManager *PlayerCameraManager;

	/** Ensure that the player is unique to the save/load system */
	virtual void OnConstruction(const FTransform &Transform) override { SaveId = FGuid(100, 100, 100, 100); };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSoftClassPtr<class UEstHUDWidget> HUDWidgetClass;

	/** Camera smooth speed used for things like crouching, jumping, and general movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraSmoothSpeed;

	/** Player camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class UCameraComponent* Camera;

	/** Player weapon container */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class USceneComponent* WeaponContainer;

	/** Player flashlight */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class USpotLightComponent* Flashlight;

	/** Player flashlight */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UEstResourceComponent* Battery;

	/** Player stamina */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UEstResourceComponent* Stamina;

	/** Player oxygen */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UEstResourceComponent* Oxygen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float StaminaGainPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float StaminaConsumptionPerSecond;

	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	virtual bool CanSprint() override;

	/** Player flashlight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	class USoundBase* FlashlightSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float FlashlightIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float FlashlightOuterConeAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float FlashlightAttenuationRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float BatteryConsumptionPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	class UCurveFloat* FlashlightFlicker;

	/** Amount of power to burst to the flashlight when it's empty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
	float FlashlightPowerBurst;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
	float ZoomFieldOfView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighting", meta = (UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
	float AimingFieldOfView;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class UPhysicsHandleComponent* CarryHandle;

	/** Skeletal mesh for the current weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fighting")
	class USceneComponent* ViewModel;

	/** Skeletal mesh for the current weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fighting")
	class USkeletalMeshComponent* ViewModelMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fighting")
	float AimInterpolationSpeed;

	/** Update speed for held objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float PlayerInteractionHeldUpdateSpeed;

	/** How far away this player can interact with things */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float PlayerInteractionDistance;

	/** Maximum distance for held objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float PlayerInteractionHeldDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float PlayerThrowLinearVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FVector PlayerThrowAngularVelocity;

	/** How much mass the player can carry in kg */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float PlayerMaximumCarryMass;

	/** Maximum radius of bounding sphere for objects player can carry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float PlayerMaximumCarryRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float SmoothZVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float VelocityDamageThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float VelocityDamageMinimum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<class UDamageType> FallDamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<class UDamageType> DrowningDamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TMap<TSubclassOf<class UDamageType>, class USoundBase*> DamageSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TMap<TSubclassOf<class UDamageType>, TSubclassOf<class UCameraShake>> DamageShakes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TMap<TSubclassOf<class UDamageType>, class UForceFeedbackEffect*> DamageForceFeedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	class USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	FLinearColor UnderwaterTintColor;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void Landed(const FHitResult& Hit);

	virtual void EquipWeapon_Implementation(AEstBaseWeapon* Weapon) override;

	virtual void UnequipWeapon_Implementation() override;

	UPROPERTY(BlueprintAssignable, Category = Hint)
	FOnShowHintDelegate OnShowHint;

	UFUNCTION(BlueprintCallable, Category = Hint)
	virtual void ShowHint(TArray<FName> Bindings, FText Label, float ShowTime, FVector WorldLocation) { OnShowHint.Broadcast(Bindings, Label, ShowTime, WorldLocation); };

	UPROPERTY(BlueprintAssignable, Category = Hint)
	FOnHideHintDelegate OnHideHint;

	UFUNCTION(BlueprintCallable, Category = Hint)
	virtual void HideHint() { OnHideHint.Broadcast(); };

	/** To be called when the player uses */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void InteractPressedInput();

	/** To be called when the player uses */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void InteractReleasedInput();

	/** Internal function to actually perform trace for interaction, returns true on hit */
	UFUNCTION()
	virtual bool DoInteractionTrace(float TraceSphereRadius, FHitResult& Result);

	/** Make the player pick up an object. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void PickUpActor(AActor* ActorToHold);

	/** Make the player drop an object. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void DropHeldActor(FVector LinearVelocity = FVector::ZeroVector, FVector AngularVelocity = FVector::ZeroVector);

	/** Is holding object */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual bool IsHoldingActor() { return HeldActor.IsValid() && !HeldActor->IsPendingKillOrUnreachable() && HeldPrimitive.IsValid() && !HeldPrimitive->IsPendingKillOrUnreachable(); }

	/** Is using object */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	virtual bool IsUsingObject() { return UsingObject.IsValid(); }

	/** Set the flashlight power level */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool AddFlashlightPower(float Power);

	/** Get the flashlight power level */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	virtual float GetFlashlightPower();

	/** Held actor */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TWeakObjectPtr<class AActor> HeldActor;

	/** Held primitive */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TWeakObjectPtr<class UPrimitiveComponent> HeldPrimitive;

	/** Held actor local bounding box (cached on pickup) */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	struct FBox HeldActorBounds;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FTransform HeldPrimitiveTransform;

	/** Actor the player is currently using */
	TWeakObjectPtr<class UObject> UsingObject;

	/** Update the held actor */
	UFUNCTION(BlueprintCallable, Category = "Tick")
	virtual void UpdateHeldActorTick(float DeltaSeconds);

	/** Update the flashlight intensity */
	UFUNCTION(BlueprintCallable, Category = "Tick")
	virtual void UpdateFlashlightTick(float DeltaSeconds);

	/** Check if there is anything in front of the player */
	UFUNCTION(BlueprintCallable, Category = "Tick")
	virtual void InFrontCheckSubTick();

	/** Check if the player is in water */
	UFUNCTION(BlueprintCallable, Category = "Tick")
	virtual void InWaterCheckSubTick();

	/** Update the flashlight intensity */
	UFUNCTION(BlueprintCallable, Category = "Tick")
	virtual void BlurFocusCheckSubTick();

	/** Update camera zoom */
	UFUNCTION(BlueprintCallable, Category = "Tick")
	virtual void UpdateZoomTick(float DeltaSeconds);

	/** Update post processing (film grain amount etc) */
	UFUNCTION(BlueprintCallable, Category = "Tick")
	virtual void UpdatePostProcessingTick(float DeltaSeconds);

	/** Update camera for crouch */
	UFUNCTION(BlueprintCallable, Category = "Tick")
	virtual void UpdateCameraTick(float DeltaSeconds);

	/** Update weapon */
	UFUNCTION(BlueprintCallable, Category = "Tick")
	virtual void UpdateViewModelTick(float DeltaSeconds);

	virtual void BeginPlay() override;

	/** Toggle flashlight */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void ToggleFlashlight();

	/** Is the player's camera currently the view target */
	UFUNCTION(BlueprintPure, Category = "Camera")
	virtual bool IsViewTarget();

	/** Show menu */
	virtual void ShowMainMenu() { ShowMenuSection(NAME_None); };

	/** Show menu section */
	virtual void ShowMenuSection(FName MenuSection);

	/** Show console menu */
	virtual void ShowConsoleMenu() { ShowMenuSection("Console"); }

	/** Show console menu */
	virtual void ShowLoadGameMenu() { ShowMenuSection("LoadGame"); }

	/** Shared logic to determine whether the load game menu should be shown on this input press. */
	virtual bool ShouldShowLoadGameMenu();

	/** Zoom out */
	virtual void SetZooming(bool Zooming);

	/** Toggle zoom */
	virtual void ToggleZoomInput(float Value);

	/** Set up the player input */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/** Input: Move forward */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveForwardInput(float Value);

	/** Input: Move right */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveRightInput(float Value);

	/** Input: Turn */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void TurnInput(float Value);

	/** Input: Look Up */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void LookUpInput(float Value);

	/** Input: Crouch */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void CrouchPressedInput();

	/** Input: Uncrouch */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void CrouchReleasedInput();

	/** Input: Jump */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void JumpPressedInput();

	/** Input: Jump */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void JumpReleasedInput();

	/** Input: Touch Begin */
	virtual void NotifyActorOnInputTouchBegin(const ETouchIndex::Type FingerIndex) override;

	/** Input: Touch End */
	virtual void NotifyActorOnInputTouchEnd(const ETouchIndex::Type FingerIndex) override;

	/** Input: Primary fire pressed */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void PrimaryAttackPressedInput();

	/** Input: Primary fire released */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void PrimaryAttackReleasedInput();

	/** Input: Secondary fire pressed */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void SecondaryAttackPressedInput();

	/** Input: Secondary fire released */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void SecondaryAttackReleasedInput();

	/** Input: Reload */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void ReloadInput();

	/** Input: Sprint pressed */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void SprintPressedInput();

	/** Input: Sprint released */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void SprintReleasedInput();

	/** Input: Move forwards pressed */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveForwardsPressedInput() { MoveForwardsAxis = 1.f; };

	/** Input: Move forwards released */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveForwardsReleasedInput() { MoveForwardsAxis = 0.f; };

	/** Input: Move backwards pressed */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveBackwardsPressedInput() { MoveBackwardsAxis = -1.f; };

	/** Input: Move backwards released */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveBackwardsReleasedInput() { MoveBackwardsAxis = 0.f; };

	/** Input: Move right pressed */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveRightPressedInput() { MoveRightAxis = 1.f; };

	/** Input: Move right released */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveRightReleasedInput() { MoveRightAxis = 0.f; };

	/** Input: Move left pressed */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveLeftPressedInput() { MoveLeftAxis = -1.f; };

	/** Input: Move left released */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void MoveLeftReleasedInput() { MoveLeftAxis = 0.f; };

	virtual void PlayPrimaryFire() override;

	virtual void PlaySecondaryFire() override;

	virtual void PlayReload() override;

	/** Holds a reference to the HUD widget */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "HUD")
	class UEstHUDWidget* HUDWidget;

	UPROPERTY()
	bool bForceCameraInterpolation;

	UPROPERTY()
	bool bZoomDesired;

	UPROPERTY()
	float RestingFieldOfView;

	UPROPERTY()
	float FlashlightTime;

	UPROPERTY()
	float MoveForwardsAxis;

	UPROPERTY()
	float MoveBackwardsAxis;

	UPROPERTY()
	float MoveRightAxis;

	UPROPERTY()
	float MoveLeftAxis;

	UPROPERTY()
	bool SomethingInFront;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool HeadInWater;

	UPROPERTY()
	bool SwimmingUp;

	UPROPERTY()
	float LastDrowningDamageTime;

	UPROPERTY()
	FHitResult FocalHit;

	UPROPERTY()
	bool bDisableDepthOfField;

	UPROPERTY()
	bool bDisableVignette;

	UPROPERTY()
	float FocalDistance;

	UPROPERTY()
	float InFrontCheckSubTickTime;

	UPROPERTY()
	float InWaterCheckSubTickTime;

	UPROPERTY()
	float BlurFocusCheckSubTickTime;

	UPROPERTY(SaveGame)
	FGuid SAVE_HeldActorSaveId;

	UPROPERTY(SaveGame)
	bool SAVE_bHasFlashlight;

	UPROPERTY(SaveGame)
	bool SAVE_bIsZooming;

	UPROPERTY(SaveGame)
	bool SAVE_bIsFlashlightOn;
};
