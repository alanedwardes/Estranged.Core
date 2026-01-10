// Estranged is a trade mark of Alan Edwardes.

#include "Volumes/EstWaterVolume.h"
#include "EstCore.h"
#include "Components/BrushComponent.h"
#include "Gameplay/EstPlayer.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Components/SphereComponent.h"
#include "Engine/Selection.h"
#include "UObject/ConstructorHelpers.h"

AEstWaterVolume::AEstWaterVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GetBrushComponent()->SetCollisionProfileName(PROFILE_WATER);
	FluidFriction = 2.f;
	bWaterVolume = true;
	bPhysicsOnContact = false;


	BuoyancyWaveFrequency = 1.5f;
	BuoyancyWaveAmplitude = 3.0f;

	AboveWaterMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("AboveWaterMesh"));
	AboveWaterMesh->SetupAttachment(GetRootComponent());
	AboveWaterMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	AboveWaterMesh->SetCastShadow(false);

	BelowWaterMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BelowWaterMesh"));
	BelowWaterMesh->SetupAttachment(GetRootComponent());
	BelowWaterMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BelowWaterMesh->SetCastShadow(false);
	BelowWaterMesh->SetRelativeRotation(FRotator(180.f, 0.f, 0.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		AboveWaterMesh->SetStaticMesh(PlaneMesh.Object);
		BelowWaterMesh->SetStaticMesh(PlaneMesh.Object);
	}

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

	if (IsValid(Manifest))
	{
		Manifest->UpdateEffects(OverlappingPlayer, GetSurfaceAt(OverlappingPlayer->GetActorLocation()));
	}

	if (IsValid(Manifest) && LastPainTime < GetWorld()->GetTimeSeconds() - Manifest->PainInterval)
	{
		CausePainTo(OverlappingPlayer);
		LastPainTime = GetWorld()->GetTimeSeconds();
	}

	if (UEstGameplayStatics::AreActorsEyesInWaterVolume(OverlappingPlayer, this))
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
				Manifest->ActivateImmersionEffects(OverlappingPlayer, GetSurfaceAt(OverlappingPlayer->GetActorLocation()));
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
				Manifest->ActivatePaddlingEffects(OverlappingPlayer, GetSurfaceAt(OverlappingPlayer->GetActorLocation()));
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
			Manifest->ActivatePaddlingEffects(OverlappingPlayer, GetSurfaceAt(OverlappingPlayer->GetActorLocation()));
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
			Manifest->ActivateImmersionEffects(OverlappingPlayer, GetSurfaceAt(OverlappingPlayer->GetActorLocation()));
		}
	}

	if (IsValid(Manifest))
	{
		if (IsValid(AboveWaterMesh))
		{
			AboveWaterMesh->SetMaterial(0, Manifest->AboveWaterMaterial);
		}

		if (IsValid(BelowWaterMesh))
		{
			BelowWaterMesh->SetMaterial(0, Manifest->BelowWaterMaterial);
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

void AEstWaterVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FBoxSphereBounds VolumeBounds = GetBrushComponent()->CalcBounds(FTransform::Identity);
	FVector VolumeExtent = VolumeBounds.BoxExtent;

	// Update Above Water Mesh
	if (AboveWaterMesh && AboveWaterMesh->GetStaticMesh())
	{
		FBoxSphereBounds MeshBounds = AboveWaterMesh->GetStaticMesh()->GetBounds();
		FVector MeshExtent = MeshBounds.BoxExtent;

		if (!FMath::IsNearlyZero(MeshExtent.X) && !FMath::IsNearlyZero(MeshExtent.Y))
		{
			FVector NewScale = FVector(VolumeExtent.X / MeshExtent.X, VolumeExtent.Y / MeshExtent.Y, 1.f);
			AboveWaterMesh->SetRelativeScale3D(NewScale);
		}

		AboveWaterMesh->SetRelativeLocation(FVector(0.f, 0.f, VolumeExtent.Z));
	}

	// Update Below Water Mesh
	if (BelowWaterMesh && BelowWaterMesh->GetStaticMesh())
	{
		FBoxSphereBounds MeshBounds = BelowWaterMesh->GetStaticMesh()->GetBounds();
		FVector MeshExtent = MeshBounds.BoxExtent;

		if (!FMath::IsNearlyZero(MeshExtent.X) && !FMath::IsNearlyZero(MeshExtent.Y))
		{
			FVector NewScale = FVector(VolumeExtent.X / MeshExtent.X, VolumeExtent.Y / MeshExtent.Y, 1.f);
			BelowWaterMesh->SetRelativeScale3D(NewScale);
		}

		BelowWaterMesh->SetRelativeLocation(FVector(0.f, 0.f, VolumeExtent.Z));
	}

	if (IsValid(Manifest))
	{
		if (IsValid(AboveWaterMesh))
		{
			AboveWaterMesh->SetMaterial(0, Manifest->AboveWaterMaterial);
		}

		if (IsValid(BelowWaterMesh))
		{
			BelowWaterMesh->SetMaterial(0, Manifest->BelowWaterMaterial);
		}
	}
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

FVector AEstWaterVolume::GetSurfaceAt(const FVector& Location) const
{
	FBoxSphereBounds Bounds = GetBrushComponent()->CalcBounds(GetBrushComponent()->GetComponentTransform());
	FVector FlatSurface = Bounds.Origin + FVector(0.f, 0.f, Bounds.BoxExtent.Z);
	// If getting surface for this volume, we assume FlatSurface Z is the base level.
	// But GetSurface() usually recalculates bounds.
	// Let's use the current GetSurface() logic but we can't call non-const GetSurface() from const function easily without duplication.
	// Duplicating the simple bounds calculation:
	FBoxSphereBounds CalcBounds = GetBrushComponent()->CalcBounds(GetBrushComponent()->GetComponentTransform());
	FlatSurface = CalcBounds.Origin + FVector(0.f, 0.f, CalcBounds.BoxExtent.Z);

	if (IsValid(Manifest) && GetWorld())
	{
		float WaveZ = Manifest->EvaluateWaveHeight(FVector(Location.X, Location.Y, FlatSurface.Z), GetWorld()->GetTimeSeconds());
		// EvaluateWaveHeight returns the absolute Z height (BaseZ + OffsetZ) ?
		// Wait, EvaluateWaveHeight implementation: return WorldPosition.Z + Offsets.Z;
		// So passing FlatSurface.Z as input Z results in FlatSurface.Z + Offset.
		return FVector(Location.X, Location.Y, WaveZ);
	}
	return FVector(Location.X, Location.Y, FlatSurface.Z);
}

void AEstWaterVolume::CausePainTo(AActor* Other)
{
	// Check if damage is really enabled
	if (FMath::IsNearlyZero(Manifest->DamagePerSec))
	{
		return;
	}

	const FVector OtherActorLocation = Other->GetActorLocation();

	// Should we always apply pain?
	bool bAlwaysApplyPain = FMath::IsNearlyZero(Manifest->PainStartRadius) && FMath::IsNearlyZero(Manifest->PainStartDepth);

	// Have we gone too far from the origin?
	bool bTooFarAway = !FMath::IsNearlyZero(Manifest->PainStartRadius) && (GetSurface() - OtherActorLocation).Size() > Manifest->PainStartRadius;

	// Are we too deep?
	// Are we too deep?
	bool bTooDeep = !FMath::IsNearlyZero(Manifest->PainStartDepth) && OtherActorLocation.Z < GetSurfaceAt(OtherActorLocation).Z - Manifest->PainStartDepth;

	if (bAlwaysApplyPain || bTooFarAway || bTooDeep)
	{
		TSubclassOf<UDamageType> DmgTypeClass = Manifest->DamageType ? *Manifest->DamageType : UDamageType::StaticClass();
		Other->TakeDamage(Manifest->DamagePerSec * Manifest->PainInterval, FDamageEvent(DmgTypeClass), nullptr, this);
	}
}