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
	AActor* FocusActor = GetBlackboardFocusActor();
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
		SetBlackboardFocusActor(FocusActor);
	}
}

void AEstAIController::OnPossess(APawn *InPawn)
{
	SetActorTickEnabled(true);
	RunBehaviorTree(BehaviorTree);

	Super::OnPossess(InPawn);
}

void AEstAIController::OnUnPossess()
{
	if (BrainComponent != nullptr)
	{
		BrainComponent->StopLogic("AEstAIController::UnPossess");
	}
	
	SetActorTickEnabled(false);

	Super::OnUnPossess();
}

void AEstAIController::SetBlackboardFocusActor(AActor *FocusActor)
{
	UBlackboardComponent* Blackboard = GetBlackboardComponent();
	if (Blackboard != nullptr)
	{
		Blackboard->SetValueAsObject(BLACKBOARD_FOCUS_ACTOR, FocusActor);
	}
}

AActor* AEstAIController::GetBlackboardFocusActor()
{
	UBlackboardComponent* Blackboard = GetBlackboardComponent();
	if (Blackboard == nullptr)
	{
		return nullptr;
	}

	UObject* FocusActor = Blackboard->GetValueAsObject(BLACKBOARD_FOCUS_ACTOR);
	if (FocusActor == nullptr)
	{
		return nullptr;
	}

	return Cast<AActor>(FocusActor);
}
