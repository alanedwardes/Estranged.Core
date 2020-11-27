// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/EstSaveRestore.h"
#include "EstSkinnedMeshCacheComponent.generated.h"

USTRUCT(BlueprintType)
struct FEstSkinnedMeshBoneState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FTransform BoneTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bIsSimulatingPhysics;
};

USTRUCT(BlueprintType)
struct FEstSkinnedMeshComponentState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName ComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bIsSimulatingPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FEstSkinnedMeshBoneState> BoneStates;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESTCORE_API UEstSkinnedMeshCacheComponent : public UActorComponent, public IEstSaveRestore
{
	GENERATED_BODY()

public:
	UEstSkinnedMeshCacheComponent(const class FObjectInitializer& ObjectInitializer);

// Begin IEstSaveRestore
	virtual void OnPreRestore_Implementation() override {};
	virtual void OnPostRestore_Implementation() override;
	virtual void OnPreSave_Implementation() override;
	virtual void OnPostSave_Implementation() override {};
// End IEstSaveRestore

	UFUNCTION(BlueprintCallable, Category = "Skinned Mesh Cache")
	virtual void RestoreSkinnedMeshStates();

	UPROPERTY(EditAnywhere, SaveGame)
	TArray<FEstSkinnedMeshComponentState> SkinnedMeshStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skinned Mesh Cache")
	bool bAutoRestore;
private:
	TArray<class USkinnedMeshComponent*> GetSkinnedMeshComponents();
	class USkinnedMeshComponent* GetSkinnedMeshComponentByName(FName ComponentName);
};
