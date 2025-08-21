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
#include "UserData/EstPhysicsUserData.h"

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
		EST_LOG(this, EEstLoggerLevel::Warning, "Parent actor is null");
		return;
	}

	TArray<UPrimitiveComponent*> Primitives;
	Owner->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		Primitive->SetNotifyRigidBodyCollision(true);
		Primitive->OnComponentHit.AddUniqueDynamic(this, &UEstPhysicsEffectsComponent::OnComponentHit);
	}

	TArray<UStaticMeshComponent*> StaticMeshComponents;
	Owner->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		UEstPhysicsUserData* PhysicsData = StaticMeshComponent->GetStaticMesh()->GetAssetUserData<UEstPhysicsUserData>();
		if (PhysicsData != nullptr)
		{
			if (PhysicsData->Mass > 0.f)
			{
				StaticMeshComponent->SetMassOverrideInKg(NAME_None, PhysicsData->Mass);
			}

			if (PhysicsData->BuoyancyCoefficient > 0.f)
			{
				MaxBuoyancyCoefficient = FMath::Max(MaxBuoyancyCoefficient, PhysicsData->BuoyancyCoefficient);
				StaticMeshComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &UEstPhysicsEffectsComponent::OnComponentBeginOverlap);
				StaticMeshComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &UEstPhysicsEffectsComponent::OnComponentEndOverlap);
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
		EST_LOG(this, EEstLoggerLevel::Warning, "Other actor is null");
		return;
	}

	CurrentPhysicsVolume = Cast<APhysicsVolume>(OtherActor);

	if (CurrentPhysicsVolume->bWaterVolume)
	{
		OverlappedComponent->SetLinearDamping(1.0f);
		OverlappedComponent->SetAngularDamping(4.0f);
	}
}

void UEstPhysicsEffectsComponent::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr)
	{
		EST_LOG(this, EEstLoggerLevel::Warning, "Other actor is null");
		return;
	}

	if (CurrentPhysicsVolume == OtherActor)
	{
		OverlappedComponent->SetLinearDamping(0.01f);
		OverlappedComponent->SetAngularDamping(0.0f);
		CurrentPhysicsVolume = nullptr;
	}
}

void UEstPhysicsEffectsComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bIsInWater = CurrentPhysicsVolume == nullptr ? false : CurrentPhysicsVolume->bWaterVolume;

	if (bIsInWater)
	{
		UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
		if (!Prim || !Prim->IsSimulatingPhysics())
			return;

		// 1. Get water surface Z
		FBoxSphereBounds WaterBounds = CurrentPhysicsVolume->GetBounds();
		float WaterLevelZ = WaterBounds.Origin.Z + WaterBounds.BoxExtent.Z;

		// 2. Get owner bounds and current velocity
		FBoxSphereBounds ActorBounds = GetOwner()->GetRootComponent()->Bounds;
		FVector BoxOrigin = ActorBounds.Origin;
		FVector BoxExtent = ActorBounds.BoxExtent;
		FVector CurrentVelocity = Prim->GetPhysicsLinearVelocity();

		float BoxTopZ = BoxOrigin.Z + BoxExtent.Z;
		float BoxBottomZ = BoxOrigin.Z - BoxExtent.Z;

		// 3. Calculate submerged height (how much of the object is underwater)
		float SubmergedHeight = FMath::Clamp(WaterLevelZ - BoxBottomZ, 0.0f, 2.0f * BoxExtent.Z);

		if (SubmergedHeight <= 0.0f)
		{
			return;
		}

		// 4. Get object mass for force balancing
		float ObjectMass = Prim->GetMass();
		if (ObjectMass <= 0.0f)
		{
			return;
		}

		// 5. Calculate buoyant force based on submerged percentage and mass
		float SubmergedPercentage = SubmergedHeight / (2.0f * BoxExtent.Z);
		float Gravity = GetWorld()->GetGravityZ();
		
		// Apply buoyant force proportional to submerged percentage
		float BuoyantForce = ObjectMass * -Gravity * SubmergedPercentage * MaxBuoyancyCoefficient * 1.5f;

		// 6. Apply damping when near water surface (reduced force when close to surface)
		float DistanceFromSurface = FMath::Abs(BoxTopZ - WaterLevelZ);
		float DampingFactor = FMath::Clamp(DistanceFromSurface / (BoxExtent.Z * 0.2f), 0.2f, 1.0f);
		
		// 7. Apply water drag (resistance) to slow down movement
		float DragCoefficient = 0.05f;
		FVector DragForce = -CurrentVelocity * DragCoefficient * ObjectMass;

		// 8. Apply forces with damping
		FVector TotalForce = FVector(0, 0, BuoyantForce * DampingFactor) + DragForce;
		Prim->AddForce(TotalForce);

		// Self-righting mechanism using quaternions to avoid gimbal lock
		FRotator CurrentRotation = Prim->GetComponentRotation();
		
		// Get current and target orientations as quaternions
		FQuat CurrentQuat = Prim->GetComponentQuat();
		FQuat TargetQuat = FQuat::MakeFromEuler(FVector(0.0f, CurrentRotation.Yaw, 0.0f));
		
		// Calculate the shortest rotation between current and target
		FQuat ErrorQuat = TargetQuat * CurrentQuat.Inverse();
		
		// Convert to axis-angle representation for torque calculation
		FVector RotationAxis;
		float RotationAngle;
		ErrorQuat.ToAxisAndAngle(RotationAxis, RotationAngle);
		
		// Normalize the rotation angle to [-π, π] range
		if (RotationAngle > PI)
		{
			RotationAngle -= 2.0f * PI;
		}
		
		// Only apply self-righting if tilted beyond a threshold (5 degrees)
		float TiltThreshold = FMath::DegreesToRadians(5.0f);
		if (FMath::Abs(RotationAngle) > TiltThreshold)
		{
			// Calculate restoring torque proportional to rotation error
			float RestoreStrength = 10000.f * ObjectMass;
			FVector RestoreTorque = RotationAxis * RotationAngle * RestoreStrength;
			
			// Apply torque directly in world space
			Prim->AddTorqueInRadians(RestoreTorque);
		}
	}
}

void UEstPhysicsEffectsComponent::OnChaosPhysicsCollision(const FChaosPhysicsCollisionInfo& CollisionInfo)
{
	UEstPhysicsCollisionHandler* Handler = Cast<UEstPhysicsCollisionHandler>(GetWorld()->PhysicsCollisionHandler);
	if (Handler == nullptr)
	{
		EST_LOG(this, EEstLoggerLevel::Warning, "Physics collision handler is null (or not the correct type)");
		return;
	}

	Handler->HandlePhysicsCollisions_AssumesLocked(CollisionInfo);
}

void UEstPhysicsEffectsComponent::OnChaosBreak(const FChaosBreakEvent& BreakEvent)
{
	UEstPhysicsCollisionHandler* Handler = Cast<UEstPhysicsCollisionHandler>(GetWorld()->PhysicsCollisionHandler);
	if (Handler == nullptr)
	{
		EST_LOG(this, EEstLoggerLevel::Warning, "Physics collision handler is null (or not the correct type)");
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
		EST_LOG(this, EEstLoggerLevel::Warning, "Parent actor is null");
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
		EST_LOG(this, EEstLoggerLevel::Warning, "Physics collision handler is null (or not the correct type)");
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
