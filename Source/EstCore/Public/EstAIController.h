#pragma once

#include "Interfaces/EstSaveRestore.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "EstGameplayStatics.h"
#include "EstAIController.generated.h"

/** The base class for all of the characters in Estranged. */
UCLASS(abstract, BlueprintType)
class ESTCORE_API AEstAIController : public AAIController, public IEstSaveRestore
{
	GENERATED_BODY()

public:
	AEstAIController(const class FObjectInitializer& ObjectInitializer);

// Begin IEstSaveRestore
	virtual void OnPreRestore_Implementation() override {};
	virtual void OnPostRestore_Implementation() override {};
	virtual void OnPreSave_Implementation() override;
	virtual void OnPostSave_Implementation() override {};
	virtual FGuid GetSaveId_Implementation() override { return SaveId; };
	virtual void OnConstruction(const FTransform &Transform) override { UEstGameplayStatics::NewGuidIfInvalid(SaveId); };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = SaveState)
	FGuid SaveId;
// End IEstSaveRestore

	virtual void BeginPlay() override;

	virtual void Possess(APawn* InPawn) override;
	virtual void UnPossess() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AI)
	UBehaviorTree* BehaviorTree;
private:
	UPROPERTY(SaveGame)
	FGuid FocusActorSaveId;
};
