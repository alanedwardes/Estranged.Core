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

	virtual void OnPreRestore_Implementation() override;
	virtual void OnPostRestore_Implementation() override;
	virtual void OnPreSave_Implementation() override;
	virtual void OnDeath_Implementation() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = Death)
	virtual void BecomeRagdoll();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Death)
	bool bDeferDeathRagdoll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Death)
	float DeferDeathRagdollTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Saving)
	bool bIsRestoring;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ragdoll)
	float RagdollLinearDamping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ragdoll)
	float RagdollAngularDamping;
private:
	UPROPERTY(SaveGame)
	FGuid ControllerSaveId;

	UPROPERTY()
	FTimerHandle DeferRagdollTimerHandle;
};
