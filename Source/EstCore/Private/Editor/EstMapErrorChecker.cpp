// Estranged is a trade mark of Alan Edwardes.

#include "Editor/EstMapErrorChecker.h"
#include "Engine.h"
#include "EstCore.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Misc/UObjectToken.h"
#include "Interfaces/EstSaveRestore.h"
#include "Misc/MapErrors.h"
#include "Engine/CollisionProfile.h"
#include "Physics/EstPhysicsEffectsComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/AssetRegistryInterface.h"

#if WITH_EDITOR
void GetDependenciesRecursive(IAssetRegistry* Registry, FName Start, TSet<TTuple<FName, FName>>& Dependencies)
{
	TArray<FName> FoundDependencies;
	Registry->GetDependencies(Start, FoundDependencies, EAssetRegistryDependencyType::Type::Hard);
	for (const FName FoundDependency : FoundDependencies)
	{
		if (FoundDependency.ToString().StartsWith("/Engine/"))
		{
			continue;
		}

		bool bIsAlreadyInSet;
		Dependencies.Add(TTuple<FName, FName>(Start, FoundDependency), &bIsAlreadyInSet);

		if (!bIsAlreadyInSet)
		{
			GetDependenciesRecursive(Registry, FoundDependency, Dependencies);
		}
	}
}


void AEstMapErrorChecker::CheckForErrors()
{
	Super::CheckForErrors();

	FMessageLog MapCheck("MapCheck");

	TSet<const UMaterialInterface*> SeenMaterials;
	TSet<const UStaticMesh*> SeenMeshes;
	TSet<const USoundBase*> SeenSounds;
	TSet<FGuid> EditorSeenSaveIds;

	auto LevelPath = GetWorld()->PersistentLevel->GetPackage()->GetFName();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	IAssetRegistry* Registry = &AssetRegistryModule.Get();

	TSet<TTuple<FName, FName>> Dependencies;
	GetDependenciesRecursive(Registry, LevelPath, Dependencies);

	for (const TTuple<FName, FName> Dependency : Dependencies)
	{
		TArray<FAssetData> ParentAssets;
		Registry->GetAssetsByPackageName(Dependency.Key, ParentAssets);
		if (ParentAssets.Num() == 0)
		{
			continue;
		}

		FAssetData Parent = ParentAssets[0];

		TArray<FAssetData> Assets;
		Registry->GetAssetsByPackageName(Dependency.Value, Assets);
		if (Assets.Num() == 0)
		{
			continue;
		}

		FAssetData Asset = Assets[0];

		if (Asset.GetClass()->IsChildOf(USoundBase::StaticClass()) && !Parent.GetClass()->IsChildOf(USoundCue::StaticClass()))
		{
			USoundBase* Sound = Cast<USoundBase>(Asset.GetAsset());

			const USoundClass* SoundClass = Sound->SoundClassObject;
			if (SoundClass != nullptr && !SoundClass->GetPathName().StartsWith("/Game/Sound/Classes/"))
			{
				MapCheck.Warning()
					->AddToken(FUObjectToken::Create(this))
					->AddToken(FTextToken::Create(FText::FromString("Found sound asset")))
					->AddToken(FUObjectToken::Create(Sound))
					->AddToken(FTextToken::Create(FText::FromString("with bad sound class")));
			}

			SeenSounds.Add(Sound);
		}
	}

	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		const AActor *Actor = *ActorItr;

		if (Actor->IsEditorOnly())
		{
			continue;
		}
		
		for (const UActorComponent* Component : Actor->GetComponents())
		{
			if (Component->IsEditorOnly())
			{
				continue;
			}

			const UAudioComponent* AudioComponent = Cast<UAudioComponent>(Component);
			if (AudioComponent != nullptr)
			{
				if (AudioComponent->SoundClassOverride != nullptr && !AudioComponent->SoundClassOverride->GetPathName().StartsWith("/Game/Sound/Classes/"))
				{
					MapCheck.Error()
						->AddToken(FUObjectToken::Create(this))
						->AddToken(FTextToken::Create(FText::FromString("Actor")))
						->AddToken(FUObjectToken::Create(Actor))
						->AddToken(FTextToken::Create(FText::FromString("has audio component")))
						->AddToken(FUObjectToken::Create(AudioComponent))
						->AddToken(FTextToken::Create(FText::FromString("with invalid sound class override")));
				}

				const USoundBase* Sound = AudioComponent->Sound;
				if (Sound != nullptr && !SeenSounds.Contains(Sound))
				{
					const USoundClass* SoundClass = Sound->SoundClassObject;
					if (SoundClass == nullptr || !SoundClass->GetPathName().StartsWith("/Game/Sound/Classes/"))
					{
						MapCheck.Warning()
							->AddToken(FUObjectToken::Create(this))
							->AddToken(FTextToken::Create(FText::FromString("Actor")))
							->AddToken(FUObjectToken::Create(Actor))
							->AddToken(FTextToken::Create(FText::FromString("has sound")))
							->AddToken(FUObjectToken::Create(Sound))
							->AddToken(FTextToken::Create(FText::FromString("with no explicit sound class (so the default will be used)")));
					}

					SeenSounds.Add(Sound);
				}
			}

			const UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Component);
			if (MeshComponent != nullptr)
			{
				const UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
				if (StaticMesh != nullptr && !SeenMeshes.Contains(StaticMesh) && !StaticMesh->GetPathName().StartsWith("/Engine/"))
				{
					const int32 Lod1Tris = StaticMesh->GetRenderData()->LODResources[0].GetNumTriangles();

					if (StaticMesh->GetNumLODs() == 1 && Lod1Tris > 2048)
					{
						MapCheck.PerformanceWarning()
							->AddToken(FUObjectToken::Create(this))
							->AddToken(FTextToken::Create(FText::FromString("Actor")))
							->AddToken(FUObjectToken::Create(Actor))
							->AddToken(FTextToken::Create(FText::FromString("has static mesh")))
							->AddToken(FUObjectToken::Create(StaticMesh))
							->AddToken(FTextToken::Create(FText::FormatOrdered(FTextFormat::FromString("which has over {0} tris, but no LOD models"), Lod1Tris)));
					}

					const int32 LastLodTris = StaticMesh->GetRenderData()->LODResources[StaticMesh->GetNumLODs() - 1].GetNumTriangles();
					if (StaticMesh->GetNumLODs() > 1 && LastLodTris > 2048)
					{
						MapCheck.PerformanceWarning()
							->AddToken(FUObjectToken::Create(this))
							->AddToken(FTextToken::Create(FText::FromString("Actor")))
							->AddToken(FUObjectToken::Create(Actor))
							->AddToken(FTextToken::Create(FText::FromString("has static mesh")))
							->AddToken(FUObjectToken::Create(StaticMesh))
							->AddToken(FTextToken::Create(FText::FormatOrdered(FTextFormat::FromString("which has LOD models, but the last has over {0} tris"), LastLodTris)));
					}

					SeenMeshes.Add(StaticMesh);
				}
			}

			const UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component);
			if (PrimitiveComponent == nullptr)
			{
				continue;
			}

			if (!PrimitiveComponent->GetCollisionEnabled())
			{
				// Only physics/collision checks from here on
				continue;
			}

			if (PrimitiveComponent->GetCollisionProfileName() == UCollisionProfile::CustomCollisionProfileName)
			{
				MapCheck.Warning()
					->AddToken(FUObjectToken::Create(this))
					->AddToken(FTextToken::Create(FText::FromString("Actor")))
					->AddToken(FUObjectToken::Create(Actor))
					->AddToken(FTextToken::Create(FText::FromString("is using custom collision profile")));
			}

			int32 NumMaterials = PrimitiveComponent->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; i++)
			{
				const UMaterialInterface* Material = PrimitiveComponent->GetMaterial(i);
				if (Material == nullptr)
				{
					continue;
				}

				if (Material->GetPathName().StartsWith("/Engine/"))
				{
					continue;
				}

				if (SeenMaterials.Contains(Material))
				{
					continue;
				}

				if (Material->GetBlendMode() == EBlendMode::BLEND_Additive)
				{
					continue;
				}

				SeenMaterials.Add(Material);

				if (Material->GetPhysicalMaterial() == nullptr || Material->GetPhysicalMaterial() == GEngine->DefaultPhysMaterial)
				{
					MapCheck.Info()
						->AddToken(FUObjectToken::Create(this))
						->AddToken(FTextToken::Create(FText::FromString("Actor")))
						->AddToken(FUObjectToken::Create(Actor))
						->AddToken(FTextToken::Create(FText::FromString("has material")))
						->AddToken(FUObjectToken::Create(Material))
						->AddToken(FTextToken::Create(FText::FromString("which does not have a valid physical material")));
				}
			}

			if (PrimitiveComponent->IsAnySimulatingPhysics())
			{
				TArray<UEstPhysicsEffectsComponent*> PhysicsEffectsComponents;
				Actor->GetComponents<UEstPhysicsEffectsComponent>(PhysicsEffectsComponents);

				if (PhysicsEffectsComponents.Num() == 0)
				{
					MapCheck.Error()
						->AddToken(FUObjectToken::Create(this))
						->AddToken(FTextToken::Create(FText::FromString("Actor")))
						->AddToken(FUObjectToken::Create(Actor))
						->AddToken(FTextToken::Create(FText::FromString("is simulating pysics, but does not have a UEstPhysicsEffectsComponent")));
				}

				if (!PrimitiveComponent->GetGenerateOverlapEvents())
				{
					MapCheck.Warning()
						->AddToken(FUObjectToken::Create(this))
						->AddToken(FTextToken::Create(FText::FromString("Actor")))
						->AddToken(FUObjectToken::Create(Actor))
						->AddToken(FTextToken::Create(FText::FromString("is simulating pysics, but does not have \"Generate Overlap Events\" enabled (meaning it can't be affected by physics push volumes)")));
				}
			}
		}
	}
}
#endif