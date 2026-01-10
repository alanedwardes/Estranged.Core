// Estranged is a trade mark of Alan Edwardes.

#include "Volumes/EstWaterVolume.h"
#include "EstCore.h"
#include "GameFramework/DamageType.h"
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

	ExcluderFadeRadius = 128.f;
	bUseWaveExcluder = false;

	AboveWaterMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("AboveWaterMesh"));
	AboveWaterMesh->SetupAttachment(GetRootComponent());
	AboveWaterMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	AboveWaterMesh->SetCastShadow(false);

	BelowWaterMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BelowWaterMesh"));
	BelowWaterMesh->SetupAttachment(GetRootComponent());
	BelowWaterMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BelowWaterMesh->SetCastShadow(false);
	BelowWaterMesh->SetReverseCulling(true);

	WaveExcluder = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("WaveExcluder"));
	WaveExcluder->SetupAttachment(GetRootComponent());
	WaveExcluder->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	WaveExcluder->SetSphereRadius(256.f);
	WaveExcluder->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		AboveWaterMesh->SetStaticMesh(PlaneMesh.Object);
		BelowWaterMesh->SetStaticMesh(PlaneMesh.Object);
	}

#if WITH_EDITORONLY_DATA
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
	Super::NotifyActorEndOverlap(OtherActor);

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

	if (IsValid(OverlappingPlayer))
	{
		if (IsValid(Manifest))
		{
			Manifest->UpdateEffects(OverlappingPlayer, GetSurfaceAt(OverlappingPlayer->GetActorLocation()));
		}

		if (IsValid(Manifest))
		{
			Manifest->UpdateEffects(OverlappingPlayer, GetSurfaceAt(OverlappingPlayer->GetActorLocation()));
		}
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

	SetMaterialParameters();
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

}

void AEstWaterVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SetMaterialParameters();
}
#endif

void AEstWaterVolume::SetMaterialParameters()
{
	if (!IsValid(Manifest))
	{
		return;
	}

	FLinearColor ExcluderParams = FLinearColor(0.f, 0.f, 0.f, 0.f);
	if (bUseWaveExcluder && IsValid(WaveExcluder))
	{
		FVector Loc = WaveExcluder->GetComponentLocation();
		ExcluderParams = FLinearColor(Loc.X, Loc.Y, WaveExcluder->GetScaledSphereRadius(), ExcluderFadeRadius);
	}

	if (IsValid(AboveWaterMesh))
	{
		AboveWaterMesh->SetColorParameterValueOnMaterials(TEXT("WaveExcluder"), ExcluderParams);
	}
	if (IsValid(BelowWaterMesh))
	{
		BelowWaterMesh->SetColorParameterValueOnMaterials(TEXT("WaveExcluder"), ExcluderParams);
	}

	for (int32 i = 0; i < 8; ++i)
	{
		FLinearColor PackedWave = FLinearColor(0.f, 0.f, 0.f, 0.f);
		if (Manifest->Waves.IsValidIndex(i))
		{
			const FEstGerstnerWave& Wave = Manifest->Waves[i];
			if (Wave.Wavelength > KINDA_SMALL_NUMBER)
			{
				const float K = 2.0f * UE_PI / Wave.Wavelength;
				PackedWave.R = Wave.Direction.X * K;
				PackedWave.G = Wave.Direction.Y * K;
				PackedWave.B = Wave.Amplitude;
				PackedWave.A = Wave.Steepness;
			}
		}

		FName ParamName = *FString::Printf(TEXT("Wave%d"), i + 1);
		if (IsValid(AboveWaterMesh))
		{
			AboveWaterMesh->SetColorParameterValueOnMaterials(ParamName, PackedWave);
		}
		if (IsValid(BelowWaterMesh))
		{
			BelowWaterMesh->SetColorParameterValueOnMaterials(ParamName, PackedWave);
		}
	}
}

void AEstWaterVolume::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetMaterialParameters();

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

	SetMaterialParameters();
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
		FVector Offsets, Normal;
		Manifest->EvaluateWaveOffsets(FVector(Location.X, Location.Y, FlatSurface.Z), GetWorld()->GetTimeSeconds(), Offsets, Normal);

		if (bUseWaveExcluder && IsValid(WaveExcluder))
		{
			FVector ExcluderPos = WaveExcluder->GetComponentLocation();
			float Dist = FVector::Dist2D(ExcluderPos, Location);
			float Radius = WaveExcluder->GetScaledSphereRadius();
			float Fade = ExcluderFadeRadius;

			// 0 inside Radius, 0-1 in Fade, 1 outside
			float Alpha = FMath::SmoothStep(Radius, Radius + Fade, Dist);
			Offsets *= Alpha;
		}

		return FVector(Location.X, Location.Y, FlatSurface.Z + Offsets.Z);
	}
	return FVector(Location.X, Location.Y, FlatSurface.Z);
}
