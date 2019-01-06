#include "EstAIController.h"
#include "EstConstants.h"
#include "Runtime/AIModule/Classes/BrainComponent.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BlackboardComponent.h"

AEstAIController::AEstAIController(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AEstAIController::OnPreSave_Implementation()
{
	AActor* FocusActor = GetFocusActor();
	if (FocusActor == nullptr)
	{
		FocusActorSaveId.Invalidate();
	}
	else if (FocusActor->GetClass()->ImplementsInterface(UEstSaveRestore::StaticClass()))
	{
		FocusActorSaveId = IEstSaveRestore::Execute_GetSaveId(FocusActor);
	}
}

void AEstAIController::BeginPlay()
{
	Super::BeginPlay();

	AActor* FocusActor = UEstGameplayStatics::FindActorBySaveIdInWorld(this, FocusActorSaveId);
	if (FocusActor != nullptr)
	{
		SetFocus(FocusActor);

		// The blackboard is the source of truth for actors which use it
		UBlackboardComponent* Blackboard = GetBlackboardComponent();
		if (Blackboard != nullptr)
		{
			Blackboard->SetValueAsObject(BLACKBOARD_FOCUS_ACTOR, FocusActor);
		}
	}
}

void AEstAIController::Possess(APawn *InPawn)
{
	SetActorTickEnabled(true);
	RunBehaviorTree(BehaviorTree);

	Super::Possess(InPawn);
}

void AEstAIController::UnPossess()
{
	if (BrainComponent != nullptr)
	{
		BrainComponent->StopLogic("AEstAIController::UnPossess");
	}
	
	SetActorTickEnabled(false);

	Super::UnPossess();
}
