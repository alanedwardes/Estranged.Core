// Estranged is a trade mark of Alan Edwardes.


#include "Gameplay/EstPlayerCheckpoint.h"
#include "Gameplay/EstSaveStatics.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "EstConstants.h"
#include "Kismet/GameplayStatics.h"
#include "Saves/EstCheckpointSave.h"
#include "Gameplay/EstGameplayStatics.h"
#include "Gameplay/EstGameInstance.h"

AEstPlayerCheckpoint::AEstPlayerCheckpoint(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	Trigger = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Trigger"));
	Trigger->SetupAttachment(GetRootComponent());
	Trigger->SetCollisionProfileName(PROFILE_TRIGGER);
	TriggerExtent = FVector(32.f, 32.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

void AEstPlayerCheckpoint::NotifyActorBeginOverlap(AActor* OtherActor)
{
	FEstCheckpoint NewCheckpoint;
	NewCheckpoint.PlayerStartTag = PlayerStartTag;
	UEstSaveStatics::AddCheckpoint(this, NewCheckpoint);

	UEstGameplayStatics::GetEstGameInstance(this)->LogMessage(FEstLoggerMessage(this, EEstLoggerLevel::Normal, FString::Printf(TEXT("AEstPlayerCheckpoint::NotifyActorBeginOverlap() - Checkpoint %s Saved"), *PlayerStartTag.ToString())));
}

#if	WITH_EDITOR
void AEstPlayerCheckpoint::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	Reposition();
}

void AEstPlayerCheckpoint::PostEditUndo()
{
	Super::PostEditUndo();
	Reposition();
}
void AEstPlayerCheckpoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Reposition();
}

void AEstPlayerCheckpoint::Reposition()
{
	Trigger->SetRelativeLocation(FVector(0.f, 0.f, TriggerExtent.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	Trigger->SetBoxExtent(FVector(TriggerExtent.X, TriggerExtent.Y, TriggerExtent.Z));
}
#endif