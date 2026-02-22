// Estranged is a trade mark of Alan Edwardes.

#include "Rendering/EstInstancedMeshArrayActor.h"
#include "Engine/CollisionProfile.h"
#include "Rendering/EstInstancedMeshArrayComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstInstancedMeshArrayActor)

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

