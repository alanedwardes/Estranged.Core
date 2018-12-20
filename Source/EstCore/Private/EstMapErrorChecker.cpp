// Estranged is a trade mark of Alan Edwardes.

#include "EstMapErrorChecker.h"
#include "EstCore.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Misc/UObjectToken.h"
#include "Misc/MapErrors.h"

#if WITH_EDITOR
void AEstMapErrorChecker::CheckForErrors()
{
	Super::CheckForErrors();

	FMessageLog MapCheck("MapCheck");

	TSet<const UMaterialInterface*> SeenMaterials;
	TSet<const UStaticMesh*> SeenMeshes;
	
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		const AActor *Actor = *ActorItr;

		if (Actor->IsEditorOnly())
		{
			continue;
		}
		
		for (const UActorComponent* Component : Actor->GetComponents())
		{
			const UPrimitiveComponent* SceneComponent = Cast<UPrimitiveComponent>(Component);
			if (SceneComponent == nullptr)
			{
				continue;
			}

			if (SceneComponent->IsEditorOnly())
			{
				continue;
			}

			const UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Component);
			if (MeshComponent != nullptr)
			{
				const UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
				if (StaticMesh && !SeenMeshes.Contains(StaticMesh) && !StaticMesh->GetPathName().StartsWith("/Engine/"))
				{
					const int32 Lod1Tris = StaticMesh->RenderData->LODResources[0].GetNumTriangles();

					if (StaticMesh->GetNumLODs() == 1 && Lod1Tris > 1024)
					{
						MapCheck.Error()
							->AddToken(FUObjectToken::Create(this))
							->AddToken(FTextToken::Create(FText::FromString("Actor")))
							->AddToken(FUObjectToken::Create(Actor))
							->AddToken(FTextToken::Create(FText::FromString("has static mesh")))
							->AddToken(FUObjectToken::Create(StaticMesh))
							->AddToken(FTextToken::Create(FText::FormatOrdered(FTextFormat::FromString("which has over {0} tris, but no LOD models"), Lod1Tris)));
					}

					const int32 LastLodTris = StaticMesh->RenderData->LODResources[StaticMesh->GetNumLODs() - 1].GetNumTriangles();
					if (StaticMesh->GetNumLODs() > 1 && LastLodTris > 1024)
					{
						MapCheck.Warning()
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

			if (!SceneComponent->GetCollisionEnabled())
			{
				continue;
			}

			int32 NumMaterials = SceneComponent->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; i++)
			{
				const UMaterialInterface* Material = SceneComponent->GetMaterial(i);
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
					MapCheck.Error()
						->AddToken(FUObjectToken::Create(this))
						->AddToken(FTextToken::Create(FText::FromString("Actor")))
						->AddToken(FUObjectToken::Create(Actor))
						->AddToken(FTextToken::Create(FText::FromString("has material")))
						->AddToken(FUObjectToken::Create(Material))
						->AddToken(FTextToken::Create(FText::FromString("which does not have a valid physical material")));
				}
			}
		}
	}
}
#endif