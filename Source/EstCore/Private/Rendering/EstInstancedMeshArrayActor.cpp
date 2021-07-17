// Estranged is a trade mark of Alan Edwardes.

#include "Engine/CollisionProfile.h"
#include "Rendering/EstInstancedMeshArrayComponent.h"
#include "Rendering/EstInstancedMeshArrayActor.h"

AEstInstancedMeshArrayActor::AEstInstancedMeshArrayActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetCanBeDamaged(false);

	InstancedMeshArrayComponent = CreateDefaultSubobject<UEstInstancedMeshArrayComponent>("InstancedMeshArrayComponent");
	InstancedMeshArrayComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	InstancedMeshArrayComponent->Mobility = EComponentMobility::Static;
	InstancedMeshArrayComponent->SetGenerateOverlapEvents(false);
	InstancedMeshArrayComponent->bUseDefaultCollision = true;

	RootComponent = InstancedMeshArrayComponent;
}

