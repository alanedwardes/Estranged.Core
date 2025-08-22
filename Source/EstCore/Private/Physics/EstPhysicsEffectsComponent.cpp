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
				ComponentUserData.Add(StaticMeshComponent, PhysicsData);
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

	APhysicsVolume* PhysicsVolume = Cast<APhysicsVolume>(OtherActor);
	if (PhysicsVolume != nullptr)
	{
		if (PhysicsVolume->bWaterVolume)
		{
			OverlappedComponent->SetLinearDamping(1.0f);
			OverlappedComponent->SetAngularDamping(4.0f);
		}

		ComponentPhysicsVolumes.Add(OverlappedComponent, MakeTuple(PhysicsVolume, Cast<AEstWaterVolume>(PhysicsVolume)));
	}
}

void UEstPhysicsEffectsComponent::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr)
	{
		EST_LOG(this, EEstLoggerLevel::Warning, "Other actor is null");
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

		ComponentPhysicsVolumes.Remove(OverlappedComponent);
	}
}

void UEstPhysicsEffectsComponent::ApplyBuoyancyForce(UPrimitiveComponent* PrimitiveComponent, APhysicsVolume* PhysicsVolume, AEstWaterVolume* WaterVolume, UEstPhysicsUserData* PhysicsUserData)
{
	if (PrimitiveComponent == nullptr || PhysicsVolume == nullptr || PhysicsUserData == nullptr)
	{
		return;
	}

	if (!PrimitiveComponent->IsSimulatingPhysics() || FMath::IsNearlyZero(PhysicsUserData->BuoyancyCoefficient))
	{
		return;
	}

	// Get water surface Z
	FBoxSphereBounds WaterBounds = PhysicsVolume->GetBounds();
	float WaterLevelZ = WaterBounds.Origin.Z + WaterBounds.BoxExtent.Z;
	
	// If we do have a "proper" water volume, get its config
	if (WaterVolume != nullptr)
	{
		// Add bobbing effect with sine wave
		float GameTime = GetWorld()->GetTimeSeconds();

		float BobbingOffset = FMath::Sin(GameTime * WaterVolume->BuoyancyWaveFrequency) * WaterVolume->BuoyancyWaveAmplitude;
		WaterLevelZ += BobbingOffset;
	}

	// Get bounds and current velocity
	FBoxSphereBounds ActorBounds = PrimitiveComponent->Bounds;
	FVector BoxOrigin = ActorBounds.Origin;
	FVector BoxExtent = ActorBounds.BoxExtent;
	FVector CurrentVelocity = PrimitiveComponent->GetPhysicsLinearVelocity();

	float BoxTopZ = BoxOrigin.Z + BoxExtent.Z;
	float BoxBottomZ = BoxOrigin.Z - BoxExtent.Z;

	// Calculate submerged height (how much of the object is underwater)
	float SubmergedHeight = FMath::Clamp(WaterLevelZ - BoxBottomZ, 0.0f, 2.0f * BoxExtent.Z);
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
	float SubmergedPercentage = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 2.0f * BoxExtent.Z), FVector2D(0.0f, 1.0f), SubmergedHeight);
	float Gravity = GetWorld()->GetGravityZ();

	// Apply buoyant force proportional to submerged percentage
	float BuoyantForce = ObjectMass * -Gravity * SubmergedPercentage * PhysicsUserData->BuoyancyCoefficient * 1.5f;

	FVector DragForce = -CurrentVelocity * ObjectMass;

	// Apply forces with drag
	FVector TotalForce = FVector(0, 0, BuoyantForce) + DragForce;
	PrimitiveComponent->AddForce(TotalForce);

	// Self-righting mechanism - find shortest path to upright or upside down
	FRotator CurrentRotation = PrimitiveComponent->GetComponentRotation();
	
	// Get current up vector
	FVector CurrentUp = CurrentRotation.RotateVector(FVector::UpVector);
	
	// Calculate angles to both upright and upside down orientations
	FVector WorldUp = FVector::UpVector;
	FVector WorldDown = -FVector::UpVector;
	
	float AngleToUp = FMath::Acos(FMath::Clamp(FVector::DotProduct(CurrentUp, WorldUp), -1.0f, 1.0f));
	float AngleToDown = FMath::Acos(FMath::Clamp(FVector::DotProduct(CurrentUp, WorldDown), -1.0f, 1.0f));
	
	// Choose the closest orientation
	FVector DesiredUp = (AngleToUp < AngleToDown) ? WorldUp : WorldDown;
	
	// Calculate the cross product to get rotation axis and sine of angle
	FVector RotationAxis = FVector::CrossProduct(CurrentUp, DesiredUp);
	float SinAngle = RotationAxis.Size();
	
	// Only apply self-righting if tilted beyond a threshold (5 degrees)
	float TiltThreshold = FMath::Sin(FMath::DegreesToRadians(5.0f));
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

void UEstPhysicsEffectsComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (TPair<UPrimitiveComponent*, TTuple<APhysicsVolume*, AEstWaterVolume*>> PrimitiveComponentPhysicsVolume : ComponentPhysicsVolumes)
	{
		UPrimitiveComponent* PrimitiveComponent = PrimitiveComponentPhysicsVolume.Key;
		APhysicsVolume* PhysicsVolume = PrimitiveComponentPhysicsVolume.Value.Key;
		AEstWaterVolume* WaterVolume = PrimitiveComponentPhysicsVolume.Value.Value;
		UEstPhysicsUserData** UserDataPtr = ComponentUserData.Find(PrimitiveComponent);

		ApplyBuoyancyForce(PrimitiveComponent, PhysicsVolume, WaterVolume, *UserDataPtr);
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
