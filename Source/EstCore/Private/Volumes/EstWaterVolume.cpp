// Estranged is a trade mark of Alan Edwardes.

#include "Volumes/EstWaterVolume.h"
#include "EstCore.h"
#include "Components/BrushComponent.h"
#include "Gameplay/EstPlayer.h"
#include "Engine/DamageEvents.h"
#include "Components/SphereComponent.h"
#include "Engine/Selection.h"

AEstWaterVolume::AEstWaterVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GetBrushComponent()->SetCollisionProfileName(PROFILE_WATER);
	FluidFriction = 0.35f;
	bWaterVolume = true;
	bPhysicsOnContact = false;

#if WITH_EDITORONLY_DATA
	PainRadius = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("PainRadius"));
	PainRadius->SetVisibility(false);
	PainRadius->SetupAttachment(GetRootComponent());
	PainRadius->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	USelection::SelectionChangedEvent.AddUObject(this, &AEstWaterVolume::OnSelectionChanged);
#endif
}

void AEstWaterVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
 	Super::NotifyActorBeginOverlap(OtherActor);

	AEstPlayer* Player = Cast<AEstPlayer>(OtherActor);
	if (IsValid(Player))
	{
		OverlappingPlayer = Player;

		SetActorTickEnabled(true);
		SetPlayerPaddling(true);
	}
}

void AEstWaterVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	AEstPlayer* Player = Cast<AEstPlayer>(OtherActor);
	if (IsValid(Player))
	{
		SetActorTickEnabled(false);
		SetPlayerImmersed(false);
		SetPlayerPaddling(false);

		OverlappingPlayer = nullptr;
	}
}

void AEstWaterVolume::Tick(float DeltaTime)
{
	if (!IsValid(OverlappingPlayer))
	{
		return;
	}

	if (Manifest != LastManifest)
	{
		ManifestChanged();
		LastManifest = Manifest;
	}

	if (IsValid(Manifest) && LastPainTime < GetWorld()->GetTimeSeconds() - Manifest->PainInterval)
	{
		CausePainTo(OverlappingPlayer);
		LastPainTime = GetWorld()->GetTimeSeconds();
	}

	if (UEstGameplayStatics::AreActorsEyesInWater(OverlappingPlayer))
	{
		SetPlayerImmersed(true);
	}
	else
	{
		SetPlayerImmersed(false);
	}
}

void AEstWaterVolume::SetPlayerImmersed(bool bEnabled)
{
	if (bEnabled)
	{
		if (!bPlayerImmersed)
		{
			if (IsValid(Manifest))
			{
				Manifest->ActivateImmersionEffects(OverlappingPlayer, GetSurface());
			}
			bPlayerImmersed = true;
		}
	}
	else
	{
		if (bPlayerImmersed)
		{
			if (IsValid(Manifest))
			{
				Manifest->DeactivateImmersionEffects(OverlappingPlayer);
			}
			bPlayerImmersed = false;
		}
	}
}

void AEstWaterVolume::SetPlayerPaddling(bool bEnabled)
{
	if (bEnabled)
	{
		if (!bPlayerPaddling)
		{
			if (IsValid(Manifest))
			{
				Manifest->ActivatePaddlingEffects(OverlappingPlayer, GetSurface());
			}
			bPlayerPaddling = true;
		}
	}
	else
	{
		if (bPlayerPaddling)
		{
			if (IsValid(Manifest))
			{
				Manifest->DeactivatePaddlingEffects(OverlappingPlayer);
			}
			bPlayerPaddling = false;
		}
	}
}

void AEstWaterVolume::ManifestChanged()
{
	if (bPlayerPaddling)
	{
		if (IsValid(LastManifest))
		{
			LastManifest->DeactivatePaddlingEffects(OverlappingPlayer);
		}

		if (IsValid(Manifest))
		{
			Manifest->ActivatePaddlingEffects(OverlappingPlayer, GetSurface());
		}
	}

	if (bPlayerImmersed)
	{
		if (IsValid(LastManifest))
		{
			LastManifest->DeactivateImmersionEffects(OverlappingPlayer);
		}

		if (IsValid(Manifest))
		{
			Manifest->ActivateImmersionEffects(OverlappingPlayer, GetSurface());
		}
	}
}

#if WITH_EDITORONLY_DATA
void AEstWaterVolume::OnSelectionChanged(UObject* NewSelection)
{
	// https://forums.unrealengine.com/t/event-when-actor-selected-in-editor/358904/5

	TArray<AEstWaterVolume*> SelectedExampleActors;

	// Get ExampleActors from the selection
	USelection* Selection = Cast<USelection>(NewSelection);
	if (Selection != nullptr)
	{
		Selection->GetSelectedObjects<AEstWaterVolume>(SelectedExampleActors);
	}

	// Search the selection for this actor
	for (AEstWaterVolume* SelectedExampleActor : SelectedExampleActors)
	{
		// If our actor is in the selection and was not previously
		// selected, then this selection change marks the actor
		// being selected
		if (SelectedExampleActor == this && !bSelectedInEditor)
		{
			// Respond to this actor being selected
			bSelectedInEditor = true;
			UpdateSelectionState();
		}
	}

	// If our record shows our actor is selected, but IsSelected() is false,
	// this selection change marks the actor being deselected
	if (bSelectedInEditor && !IsSelected())
	{
		// Respond to this actor being deselected
		bSelectedInEditor = false;
		UpdateSelectionState();
	}
}
void AEstWaterVolume::UpdateSelectionState()
{
	if (IsValid(Manifest))
	{
		PainRadius->SetSphereRadius(Manifest->PainStartRadius);
	}

	PainRadius->SetWorldLocation(GetSurface());
	PainRadius->SetVisibility(bSelectedInEditor);
}
#endif

void AEstWaterVolume::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	LastManifest = Manifest;
}

void AEstWaterVolume::ActorEnteredVolume(AActor* Other)
{
	Super::ActorEnteredVolume(Other);
}

void AEstWaterVolume::ActorLeavingVolume(AActor* Other)
{
	Super::ActorLeavingVolume(Other);
}

FVector AEstWaterVolume::GetSurface()
{
	FBoxSphereBounds Bounds = GetBrushComponent()->CalcBounds(GetBrushComponent()->GetComponentTransform());
	return Bounds.Origin + FVector(0.f, 0.f, Bounds.BoxExtent.Z);
}

void AEstWaterVolume::CausePainTo(AActor* Other)
{
	// Check if damage is really enabled
	if (FMath::IsNearlyZero(Manifest->DamagePerSec))
	{
		return;
	}

	const FVector OtherActorLocation = Other->GetActorLocation();

	// Have we gone too far from the origin?
	bool bTooFarAway = !FMath::IsNearlyZero(Manifest->PainStartRadius) && (GetSurface() - OtherActorLocation).Size() > Manifest->PainStartRadius;

	// Are we too deep?
	bool bTooDeep = !FMath::IsNearlyZero(Manifest->PainStartDepth) && OtherActorLocation.Z < GetSurface().Z - Manifest->PainStartDepth;

	if (bTooFarAway || bTooDeep)
	{
		TSubclassOf<UDamageType> DmgTypeClass = Manifest->DamageType ? *Manifest->DamageType : UDamageType::StaticClass();
		Other->TakeDamage(Manifest->DamagePerSec * Manifest->PainInterval, FDamageEvent(DmgTypeClass), nullptr, this);
	}
}