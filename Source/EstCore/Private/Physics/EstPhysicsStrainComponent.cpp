// Estranged is a trade mark of Alan Edwardes.

#include "Physics/EstPhysicsStrainComponent.h"
#include "Field/FieldSystemComponent.h"

UEstPhysicsStrainComponent::UEstPhysicsStrainComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEstPhysicsStrainComponent::OnRegister()
{
	Super::OnRegister();

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	PrimitiveComponent = Cast<UPrimitiveComponent>(Owner->FindComponentByClass<UPrimitiveComponent>());
	if (PrimitiveComponent == nullptr)
	{
		return;
	}

	FieldComponent = Owner->FindComponentByClass<UFieldSystemComponent>();
	if (FieldComponent == nullptr)
	{
		FieldComponent = NewObject<UFieldSystemComponent>(Owner);
		FieldComponent->AttachToComponent(PrimitiveComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		FieldComponent->RegisterComponent();
	}
}

void UEstPhysicsStrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (Owner == nullptr || FieldComponent == nullptr || PrimitiveComponent == nullptr)
	{
		return;
	}

	const float Mass = PrimitiveComponent->GetMass();
	const float Magnitude = FMath::Pow(2.f, 512.f); // TODO: Scale using mass
	FieldComponent->ApplyStrainField(true, PrimitiveComponent->Bounds.Origin, PrimitiveComponent->Bounds.SphereRadius * 2.f, Magnitude, 0);
}

void UEstPhysicsStrainComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	FieldComponent->DestroyComponent();
}
