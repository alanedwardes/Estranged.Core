// Estranged is a trade mark of Alan Edwardes.


#include "Physics/EstPhysicsEffectsComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Physics/EstPhysicsCollisionHandler.h"
#include "PBDRigidsSolver.h"
#include "GameFramework/PhysicsVolume.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Gameplay/EstGameplayStatics.h"
#include "Gameplay/EstGameInstance.h"
#include "Volumes/EstWaterVolume.h"
#include "UserData/EstPhysicsUserData.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstPhysicsEffectsComponent)

DEFINE_LOG_CATEGORY(LogEstPhysicsEffectsComponent);

UEstPhysicsEffectsComponent::UEstPhysicsEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEstPhysicsEffectsComponent::OnRegister()
{
	// WARNING: This method MUST be idempotent, since it is executed multiple times.
	// For example: it is executed in the editor, then in PIE.
	// Use AddUniqueDynamic instead of AddDynamic for delegates.

	Super::OnRegister();

	bool bShouldTick = false;

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		EST_LOG(this, Warning, "Parent actor is null");
		return;
	}

	TArray<UPrimitiveComponent*> Primitives;
	Owner->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(Primitive);
		if (SkelMesh != nullptr)
		{
			// Skeletal meshes need collision notifications enabled on all bodies
			SkelMesh->SetAllBodiesNotifyRigidBodyCollision(true);
		}
		else
		{
			// This works for static meshes and other primitive components
			Primitive->SetNotifyRigidBodyCollision(true);
		}

		Primitive->SetGenerateOverlapEvents(true);
		Primitive->SetReceivesDecals(false);
		Primitive->OnComponentHit.AddUniqueDynamic(this, &UEstPhysicsEffectsComponent::OnComponentHit);

		// This path looks for physics user data on the primitive component - could be a static mesh, skeletal mesh, etc
		UEstPhysicsUserData* PhysicsData = UEstGameplayStatics::GetUserDataFromMesh<UEstPhysicsUserData>(Primitive);
		if (PhysicsData != nullptr)
		{
			if (PhysicsData->Mass > 0.f)
			{
				Primitive->SetMassOverrideInKg(NAME_None, PhysicsData->Mass);
			}

			if (PhysicsData->BuoyancyCoefficient > 0.f)
			{
				ComponentUserData.Add(Primitive, PhysicsData);
				Primitive->OnComponentBeginOverlap.AddUniqueDynamic(this, &UEstPhysicsEffectsComponent::OnComponentBeginOverlap);
				Primitive->OnComponentEndOverlap.AddUniqueDynamic(this, &UEstPhysicsEffectsComponent::OnComponentEndOverlap);
				bShouldTick = true;
			}
		}
	}

	TArray<UGeometryCollectionComponent*> GeometryCollectionComponents;
	Owner->GetComponents<UGeometryCollectionComponent>(GeometryCollectionComponents);
	for (UGeometryCollectionComponent* GeometryCollectionComponent : GeometryCollectionComponents)
	{
		GeometryCollectionComponent->bNotifyCollisions = true;
		GeometryCollectionComponent->bNotifyBreaks = true;
		GeometryCollectionComponent->OnChaosBreakEvent.AddUniqueDynamic(this, &UEstPhysicsEffectsComponent::OnChaosBreak);
		GeometryCollectionComponent->OnChaosPhysicsCollision.AddUniqueDynamic(this, &UEstPhysicsEffectsComponent::OnChaosPhysicsCollision);
	}

	SetComponentTickEnabled(bShouldTick);
}

void UEstPhysicsEffectsComponent::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr)
	{
		EST_LOG(this, Warning, "Other actor is null");
		return;
	}

	APhysicsVolume* PhysicsVolume = Cast<APhysicsVolume>(OtherActor);
	if (PhysicsVolume != nullptr)
	{
		if (PhysicsVolume->bWaterVolume)
		{
			OverlappedComponent->SetLinearDamping(1.0f);
			OverlappedComponent->SetAngularDamping(4.0f);
		}

		ComponentPhysicsVolumes.Add(FPhyicsEffectsPhysicsVolumeInfo(OverlappedComponent, PhysicsVolume, Cast<AEstWaterVolume>(PhysicsVolume)));
	}
}

void UEstPhysicsEffectsComponent::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr)
	{
		EST_LOG(this, Warning, "Other actor is null");
		return;
	}

	APhysicsVolume* PhysicsVolume = Cast<APhysicsVolume>(OtherActor);
	if (PhysicsVolume != nullptr)
	{
		if (PhysicsVolume->bWaterVolume)
		{
			OverlappedComponent->SetLinearDamping(0.01f);
			OverlappedComponent->SetAngularDamping(0.0f);
		}

		ComponentPhysicsVolumes.Remove(FPhyicsEffectsPhysicsVolumeInfo(OverlappedComponent, PhysicsVolume, Cast<AEstWaterVolume>(PhysicsVolume)));
	}
}

void UEstPhysicsEffectsComponent::ApplyBuoyancyForce(UPrimitiveComponent* PrimitiveComponent, APhysicsVolume* PhysicsVolume, AEstWaterVolume* WaterVolume, UEstPhysicsUserData* PhysicsUserData)
{
	if (PrimitiveComponent == nullptr || PhysicsVolume == nullptr || PhysicsUserData == nullptr)
	{
		return;
	}

	if (!PrimitiveComponent->IsSimulatingPhysics() || FMath::IsNearlyZero(PhysicsUserData->BuoyancyCoefficient) || !PhysicsVolume->bWaterVolume)
	{
		return;
	}

	// Get water surface Z and Normal
	FBoxSphereBounds WaterBounds = PhysicsVolume->GetBounds();
	float WaterLevelZ = WaterBounds.Origin.Z + WaterBounds.BoxExtent.Z;
	FVector SurfaceNormal = FVector::UpVector;

	// Get bounds and current velocity
	FBoxSphereBounds ActorBounds = PrimitiveComponent->Bounds;
	FVector BoxOrigin = ActorBounds.Origin;
	FVector BoxExtent = ActorBounds.BoxExtent;
	FVector CurrentVelocity = PrimitiveComponent->GetPhysicsLinearVelocity();
	
	// If we do have a "proper" water volume, get its config
	if (WaterVolume != nullptr)
	{
		// Override water level and normal with water volume surface (accounts for waves)
		FVector SurfacePos;
		WaterVolume->GetSurfaceData(BoxOrigin, SurfacePos, SurfaceNormal);
		WaterLevelZ = SurfacePos.Z;
	}

	// Apply hull offset relative to the object's current orientation
	// This allows boats to float with their hull at water surface instead of center
	// The offset is applied in the object's local "down" direction
	FRotator CurrentRotation = PrimitiveComponent->GetComponentRotation();
	FVector LocalDown = CurrentRotation.RotateVector(-FVector::UpVector);
	FVector HullOffsetVector = LocalDown * PhysicsUserData->HullOffset;
	
	// Calculate effective bounds with orientation-aware hull offset
	float EffectiveBottomZ = BoxOrigin.Z - BoxExtent.Z + HullOffsetVector.Z;
	float EffectiveTopZ = BoxOrigin.Z + BoxExtent.Z + HullOffsetVector.Z;
	float EffectiveHeight = 2.0f * BoxExtent.Z;

	// Calculate submerged height based on the hull-adjusted bottom
	float SubmergedHeight = FMath::Clamp(WaterLevelZ - EffectiveBottomZ, 0.0f, EffectiveHeight);
	if (SubmergedHeight <= 0.0f)
	{
		return;
	}

	// Get object mass for force balancing
	float ObjectMass = PrimitiveComponent->GetMass();
	if (ObjectMass <= 0.0f)
	{
		return;
	}

	// Calculate buoyant force based on submerged percentage and mass
	float SubmergedPercentage = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, EffectiveHeight), FVector2D(0.0f, 1.0f), SubmergedHeight);
	float Gravity = GetWorld()->GetGravityZ();

	// Apply buoyant force proportional to submerged percentage, directed along the surface normal
	// This makes objects align with the wave surface and also provides some lateral push/sliding
	float BuoyantForceMagnitude = ObjectMass * -Gravity * SubmergedPercentage * PhysicsUserData->BuoyancyCoefficient;
	FVector BuoyantForce = SurfaceNormal * BuoyantForceMagnitude;

	FVector DragForce = -CurrentVelocity * ObjectMass;

	// Apply forces with drag
	FVector TotalForce = BuoyantForce + DragForce;
	PrimitiveComponent->AddForce(TotalForce);

	if (!FMath::IsNearlyZero(PhysicsUserData->SelfRightingDegrees))
	{	
		// Get current up vector
		FVector CurrentUp = CurrentRotation.RotateVector(FVector::UpVector);
		
		// Use surface normal as the target up vector for the rolling effect
		FVector TargetUp = SurfaceNormal;
		FVector TargetDown = -SurfaceNormal;
		
		float AngleToUp = FMath::Acos(FMath::Clamp(FVector::DotProduct(CurrentUp, TargetUp), -1.0f, 1.0f));
		float AngleToDown = FMath::Acos(FMath::Clamp(FVector::DotProduct(CurrentUp, TargetDown), -1.0f, 1.0f));
		
		// Choose the closest orientation (upright or upside down relative to wave slope)
		FVector DesiredUp = (AngleToUp < AngleToDown) ? TargetUp : TargetDown;
		
		// Calculate the cross product to get rotation axis and sine of angle
		FVector RotationAxis = FVector::CrossProduct(CurrentUp, DesiredUp);
		float SinAngle = RotationAxis.Size();
		
		// Only apply self-righting if tilted beyond a threshold
		float TiltThreshold = FMath::Sin(FMath::DegreesToRadians(PhysicsUserData->SelfRightingDegrees));
		if (SinAngle > TiltThreshold)
		{
			// Normalize the rotation axis
			RotationAxis = RotationAxis.GetSafeNormal(0.0f);
			
			// Calculate angle from sin
			float RotationAngle = FMath::Asin(FMath::Clamp(SinAngle, 0.0f, 1.0f));
			
			// Calculate restoring torque proportional to rotation error
			float RestoreStrength = 10000.f * ObjectMass;
			FVector RestoreTorque = RotationAxis * RotationAngle * RestoreStrength;

			// Apply torque directly in world space
			PrimitiveComponent->AddTorqueInRadians(RestoreTorque);
		}
	}
}

void UEstPhysicsEffectsComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (const FPhyicsEffectsPhysicsVolumeInfo PhyicsEffectsPhysicsVolumeInfo : ComponentPhysicsVolumes)
	{
		UEstPhysicsUserData** UserDataPtr = ComponentUserData.Find(PhyicsEffectsPhysicsVolumeInfo.PrimitiveComponent);
		UEstPhysicsUserData* UserData = UserDataPtr == nullptr ? nullptr : *UserDataPtr;

		ApplyBuoyancyForce(PhyicsEffectsPhysicsVolumeInfo.PrimitiveComponent, PhyicsEffectsPhysicsVolumeInfo.PhysicsVolume, PhyicsEffectsPhysicsVolumeInfo.WaterVolume, UserData);
	}
}

void UEstPhysicsEffectsComponent::OnChaosPhysicsCollision(const FChaosPhysicsCollisionInfo& CollisionInfo)
{
	UEstPhysicsCollisionHandler* Handler = Cast<UEstPhysicsCollisionHandler>(GetWorld()->PhysicsCollisionHandler);
	if (Handler == nullptr)
	{
		EST_LOG(this, Warning, "Physics collision handler is null (or not the correct type)");
		return;
	}

	Handler->HandlePhysicsCollisions_AssumesLocked(CollisionInfo);
}

void UEstPhysicsEffectsComponent::OnChaosBreak(const FChaosBreakEvent& BreakEvent)
{
	UEstPhysicsCollisionHandler* Handler = Cast<UEstPhysicsCollisionHandler>(GetWorld()->PhysicsCollisionHandler);
	if (Handler == nullptr)
	{
		EST_LOG(this, Warning, "Physics collision handler is null (or not the correct type)");
		return;
	}

	Handler->HandlePhysicsBreak_AssumesLocked(BreakEvent);
}

void UEstPhysicsEffectsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		EST_LOG(this, Warning, "Parent actor is null");
		return;
	}

	TArray<UPrimitiveComponent*> Primitives;
	Owner->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		Primitive->OnComponentHit.RemoveDynamic(this, &UEstPhysicsEffectsComponent::OnComponentHit);
	}
}

void UEstPhysicsEffectsComponent::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UEstPhysicsCollisionHandler* Handler = Cast<UEstPhysicsCollisionHandler>(GetWorld()->PhysicsCollisionHandler);
	if (Handler == nullptr)
	{
		EST_LOG(this, Warning, "Physics collision handler is null (or not the correct type)");
		return;
	}

	FRigidBodyCollisionInfo MyInfo = FRigidBodyCollisionInfo();
	MyInfo.SetFrom(HitComponent->GetBodyInstance());
	FRigidBodyCollisionInfo OtherInfo = FRigidBodyCollisionInfo();
	OtherInfo.SetFrom(OtherComp->GetBodyInstance());

	FRigidBodyContactInfo ContactInfo = FRigidBodyContactInfo();
	ContactInfo.ContactPosition = Hit.ImpactPoint;
	ContactInfo.ContactNormal = Hit.ImpactNormal;

	FCollisionImpactData RigidCollisionData = FCollisionImpactData();
	RigidCollisionData.ContactInfos.Add(ContactInfo);

	Handler->CustomHandleCollision_AssumesLocked(MyInfo, OtherInfo, RigidCollisionData);
}
