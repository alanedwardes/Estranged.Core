// Estranged is a trade mark of Alan Edwardes.


#include "Gameplay/EstPlayerCheckpoint.h"
#include "Gameplay/EstSaveStatics.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "EstConstants.h"
#include "Kismet/GameplayStatics.h"
#include "Saves/EstCheckpointSave.h"

AEstPlayerCheckpoint::AEstPlayerCheckpoint(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	Trigger = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Trigger"));
	Trigger->SetupAttachment(GetRootComponent());
	Trigger->SetCollisionProfileName(PROFILE_TRIGGER);
}

void AEstPlayerCheckpoint::NotifyActorBeginOverlap(AActor* OtherActor)
{
	FEstCheckpoint NewCheckpoint;
	NewCheckpoint.Level = UGameplayStatics::GetCurrentLevelName(this);
	NewCheckpoint.PlayerStartTag = PlayerStartTag;
	NewCheckpoint.CreatedOn = FDateTime::UtcNow();
	UEstSaveStatics::AddCheckpoint(NewCheckpoint);

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Checkpoint Saved"));
}
