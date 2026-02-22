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
#include "DrawDebugHelpers.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstWaterVolume)

#define WAVE_ECLUDER_MATERIAL_PARAMETER "WaveExcluder"
#define WAVE_INTENSITY_MATERIAL_PARAMETER "WaveIntensity"

AEstWaterVolume::AEstWaterVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GetBrushComponent()->SetCollisionProfileName(PROFILE_WATER);
	FluidFriction = 2.f;
	bWaterVolume = true;
	bPhysicsOnContact = false;

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

	if (IsValid(Manifest))
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		OverlappingPlayer->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		Manifest->UpdateEffects(OverlappingPlayer, GetSurfaceAt(EyeLocation), AboveWaterMesh->GetComponentLocation(), WaveIntensity);

		if (IsValid(AboveWaterMesh))
		{
			AboveWaterMesh->SetScalarParameterValueOnMaterials(WAVE_INTENSITY_MATERIAL_PARAMETER, WaveIntensity);
		}
		if (IsValid(BelowWaterMesh))
		{
			BelowWaterMesh->SetScalarParameterValueOnMaterials(WAVE_INTENSITY_MATERIAL_PARAMETER, WaveIntensity);
		}

		// Debug draw CPU wave simulation as a point grid
		if (bDebugDrawCPUWaves && DebugGridSpacing > 0.f)
		{
			const float HalfSize = DebugGridSize * 0.5f;
			const FVector PlayerPos = OverlappingPlayer->GetActorLocation();
			
			for (float X = -HalfSize; X <= HalfSize; X += DebugGridSpacing)
			{
				for (float Y = -HalfSize; Y <= HalfSize; Y += DebugGridSpacing)
				{
					FVector SamplePos = FVector(PlayerPos.X + X, PlayerPos.Y + Y, 0.f);
					FVector SurfacePos, SurfaceNormal;
					GetSurfaceData(SamplePos, SurfacePos, SurfaceNormal);

					DrawDebugPoint(GetWorld(), SurfacePos, 5.f, FColor::Red, false, -1.f, 0);
					DrawDebugLine(GetWorld(), SurfacePos, SurfacePos + (SurfaceNormal * 50.f), FColor::Blue, false, -1.f, 0, 1.f);
				}
			}
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
				Manifest->ActivateImmersionEffects(OverlappingPlayer);
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
				Manifest->ActivatePaddlingEffects(OverlappingPlayer);
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
			Manifest->ActivatePaddlingEffects(OverlappingPlayer);
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
			Manifest->ActivateImmersionEffects(OverlappingPlayer);
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
		}
	}

	// If our record shows our actor is selected, but IsSelected() is false,
	// this selection change marks the actor being deselected
	if (bSelectedInEditor && !IsSelected())
	{
		// Respond to this actor being deselected
		bSelectedInEditor = false;
	}
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
		AboveWaterMesh->SetColorParameterValueOnMaterials(WAVE_ECLUDER_MATERIAL_PARAMETER, ExcluderParams);
		AboveWaterMesh->SetScalarParameterValueOnMaterials(WAVE_INTENSITY_MATERIAL_PARAMETER, WaveIntensity);
	}
	if (IsValid(BelowWaterMesh))
	{
		BelowWaterMesh->SetColorParameterValueOnMaterials(WAVE_ECLUDER_MATERIAL_PARAMETER, ExcluderParams);
		BelowWaterMesh->SetScalarParameterValueOnMaterials(WAVE_INTENSITY_MATERIAL_PARAMETER, WaveIntensity);
	}

	const FEstGerstnerWave* WavePtrs[] = { &Manifest->Wave1, &Manifest->Wave2, &Manifest->Wave3, &Manifest->Wave4, &Manifest->Wave5, &Manifest->Wave6, &Manifest->Wave7, &Manifest->Wave8 };

	for (int32 i = 0; i < 8; ++i)
	{
		FLinearColor PackedWave = FLinearColor(0.f, 0.f, 0.f, 0.f);
		const FEstGerstnerWave& Wave = *WavePtrs[i];
		
		if (Wave.Wavelength > KINDA_SMALL_NUMBER)
		{
			PackedWave.R = Wave.Angle;
			PackedWave.G = Wave.Wavelength;
			PackedWave.B = Wave.Amplitude;
			PackedWave.A = Wave.Steepness;
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
			FVector NewScale = FVector(VolumeExtent.X / MeshExtent.X, VolumeExtent.Y / MeshExtent.Y, -1.f);
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
	FVector SurfacePos, SurfaceNormal;
	GetSurfaceData(Location, SurfacePos, SurfaceNormal);
	return SurfacePos;
}

void AEstWaterVolume::GetSurfaceData(const FVector& Location, FVector& OutSurfacePosition, FVector& OutSurfaceNormal) const
{
	FBoxSphereBounds CalcBounds = GetBrushComponent()->CalcBounds(GetBrushComponent()->GetComponentTransform());
	FVector FlatSurface = CalcBounds.Origin + FVector(0.f, 0.f, CalcBounds.BoxExtent.Z);

	OutSurfacePosition = FVector(Location.X, Location.Y, FlatSurface.Z);
	OutSurfaceNormal = FVector::UpVector;

	if (IsValid(Manifest) && GetWorld())
	{
		FVector Offsets, Normal;
		Manifest->EvaluateWaveOffsets(FVector(Location.X, Location.Y, FlatSurface.Z), GetWorld()->GetTimeSeconds(), Offsets, Normal, WaveIntensity);

		if (bUseWaveExcluder && IsValid(WaveExcluder))
		{
			FVector ExcluderPos = WaveExcluder->GetComponentLocation();
			float Dist = FVector::Dist2D(ExcluderPos, Location);
			float Radius = WaveExcluder->GetScaledSphereRadius();
			float Fade = ExcluderFadeRadius;

			// 0 inside Radius, 0-1 in Fade, 1 outside
			float Alpha = FMath::SmoothStep(Radius, Radius + Fade, Dist);
			Offsets *= Alpha;
			
			// Interpolate normal between world up and wave normal
			Normal = FMath::Lerp(FVector::UpVector, Normal, Alpha).GetSafeNormal();
		}

		OutSurfacePosition.Z += Offsets.Z;
		OutSurfaceNormal = Normal;
	}
}
