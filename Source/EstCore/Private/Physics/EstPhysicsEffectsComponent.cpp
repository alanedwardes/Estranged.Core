// Estranged is a trade mark of Alan Edwardes.


#include "Physics/EstPhysicsEffectsComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Physics/EstPhysicsCollisionHandler.h"
#include "PBDRigidsSolver.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

DEFINE_LOG_CATEGORY(LogEstPhysicsEffectsComponent);

UEstPhysicsEffectsComponent::UEstPhysicsEffectsComponent()
{
}

void UEstPhysicsEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Parent actor is null"));
		return;
	}

	TArray<UPrimitiveComponent*> Primitives;
	Owner->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		Primitive->SetNotifyRigidBodyCollision(true);
		Primitive->OnComponentHit.AddDynamic(this, &UEstPhysicsEffectsComponent::OnComponentHit);
	}

	// Must be done first, see https://alanedwardes.com/blog/posts/enabling-ue5-chaos-events/
	Chaos::FPhysicsSolver* Solver = GetWorld()->GetPhysicsScene()->GetSolver();
	Solver->EnqueueCommandImmediate([Solver]()
	{
		Solver->SetGenerateBreakingData(true);
		Solver->SetGenerateCollisionData(true);
	});

	TArray<UGeometryCollectionComponent*> GeometryCollectionComponents;
	Owner->GetComponents<UGeometryCollectionComponent>(GeometryCollectionComponents);
	for (UGeometryCollectionComponent* GeometryCollectionComponent : GeometryCollectionComponents)
	{
		GeometryCollectionComponent->SetNotifyBreaks(true);
		GeometryCollectionComponent->OnChaosBreakEvent.AddDynamic(this, &UEstPhysicsEffectsComponent::OnChaosBreak);
		GeometryCollectionComponent->OnChaosPhysicsCollision.AddDynamic(this, &UEstPhysicsEffectsComponent::OnChaosPhysicsCollision);
		
	}
}

void UEstPhysicsEffectsComponent::OnChaosPhysicsCollision(const FChaosPhysicsCollisionInfo& CollisionInfo)
{
	UEstPhysicsCollisionHandler* Handler = Cast<UEstPhysicsCollisionHandler>(GetWorld()->PhysicsCollisionHandler);
	if (Handler == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Physics collision handler is null (or not the correct type)"));
		return;
	}

	Handler->HandlePhysicsCollisions_AssumesLocked(CollisionInfo);
}

void UEstPhysicsEffectsComponent::OnChaosBreak(const FChaosBreakEvent& BreakEvent)
{
	UEstPhysicsCollisionHandler* Handler = Cast<UEstPhysicsCollisionHandler>(GetWorld()->PhysicsCollisionHandler);
	if (Handler == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Physics collision handler is null (or not the correct type)"));
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
		UE_LOG(LogTemp, Warning, TEXT("Parent actor is null"));
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
		UE_LOG(LogTemp, Warning, TEXT("Physics collision handler is null (or not the correct type)"));
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
