#include "EstAICharacter.h"
#include "EstConstants.h"
#include "EstGameplayStatics.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "EstHealthComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Controller.h"
#include "Runtime/Engine/Public/TimerManager.h"

AEstAICharacter::AEstAICharacter(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AutoPossessAI = EAutoPossessAI::Disabled;
	DeferDeathRagdollTime = .5f;
}

void AEstAICharacter::OnPreRestore_Implementation()
{
	bIsRestoring = true;
}

void AEstAICharacter::OnPostRestore_Implementation()
{
	Super::OnPostRestore_Implementation();

	if (!HealthComponent->IsDepleted())
	{
		AController* Controller = UEstGameplayStatics::FindActorBySaveIdInWorld<AController>(this, ControllerSaveId);
		if (Controller != nullptr)
		{
			Controller->Possess(this);
		}
	}

	bIsRestoring = false;
}

void AEstAICharacter::OnPreSave_Implementation()
{
	Super::OnPreSave_Implementation();

	if (Controller == nullptr)
	{
		ControllerSaveId.Invalidate();
	}
	else if (Controller->GetClass()->ImplementsInterface(UEstSaveRestore::StaticClass()))
	{
		ControllerSaveId = IEstSaveRestore::Execute_GetSaveId(Controller);
	}
}

void AEstAICharacter::OnDeath_Implementation()
{
	Super::OnDeath_Implementation();

	if (Controller != nullptr)
	{
		AController* CurrentController = Controller;
		CurrentController->UnPossess();
		CurrentController->Destroy();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (bDeferDeathRagdoll && !bIsRestoring)
	{
		GetWorldTimerManager().SetTimer(DeferRagdollTimerHandle, this, &AEstAICharacter::BecomeRagdoll, DeferDeathRagdollTime);
	}
	else
	{
		BecomeRagdoll();
	}

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionProfileName(PROFILE_DEBRIS);
}

void AEstAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Controller == nullptr && !HealthComponent->IsDepleted())
	{
		SpawnDefaultController();
	}
}

void AEstAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DeferRagdollTimerHandle.IsValid())
	{
		DeferRagdollTimerHandle.Invalidate();
	}

	Super::EndPlay(EndPlayReason);
}

void AEstAICharacter::BecomeRagdoll()
{
	GetMesh()->SetAnimation(nullptr);
	GetMesh()->SetAnimInstanceClass(nullptr);
	GetMesh()->SetSimulatePhysics(true);
}
