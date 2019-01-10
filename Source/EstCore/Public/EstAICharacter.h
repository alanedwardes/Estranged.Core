#pragma once

#include "Interfaces/EstSaveRestore.h"
#include "EstBaseCharacter.h"
#include "EstAICharacter.generated.h"

/** The base class for all of the characters in Estranged. */
UCLASS(abstract, BlueprintType)
class ESTCORE_API AEstAICharacter : public AEstBaseCharacter
{
	GENERATED_BODY()

public:
	AEstAICharacter(const class FObjectInitializer& ObjectInitializer);

	void OnPostRestore_Implementation() override;
	void OnPreSave_Implementation() override;
	void OnDeath_Implementation() override;
	void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Death)
	bool bClearAnimInstanceOnDeath;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Death)
	bool bClearAnimationOnDeath;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Death)
	bool bSimulatePhysicsOnDeath;
private:
	UPROPERTY(SaveGame)
	FGuid ControllerSaveId;
};
