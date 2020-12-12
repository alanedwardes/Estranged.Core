// Estranged is a trade mark of Alan Edwardes.

#include "Gameplay/EstGameMode.h"
#include "Engine/Engine.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/EstPreBeginPlay.h"
#include "Interfaces/EstPlayerSpawn.h"

void AEstGameMode::HandleMatchHasStarted()
{
	GameSession->HandleMatchHasStarted();

	// start human players first
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && (PlayerController->GetPawn() == nullptr) && PlayerCanRestart(PlayerController))
		{
			RestartPlayer(PlayerController);
		}
	}

	// Make sure level streaming is up to date before triggering NotifyMatchStarted
	GEngine->BlockTillLevelStreamingCompleted(GetWorld());

	OnPreBeginPlay();

	// First fire BeginPlay, if we haven't already in waiting to start match
	GetWorldSettings()->NotifyBeginPlay();

	// Then fire off match started
	GetWorldSettings()->NotifyMatchStarted();

	if (IsHandlingReplays() && GetGameInstance() != nullptr)
	{
		GetGameInstance()->StartRecordingReplay(GetWorld()->GetMapName(), GetWorld()->GetMapName());
	}
}

void AEstGameMode::OnPreBeginPlay_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor->GetClass()->ImplementsInterface(UEstPreBeginPlay::StaticClass()))
			{
				IEstPreBeginPlay::Execute_OnPreBeginPlay(Actor);
			}
		}
	}
}

void AEstGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	ACharacter* NewCharacter = NewPlayer->GetCharacter();

	FVector PlayerOrigin;
	FVector PlayerExtent;
	NewCharacter->GetActorBounds(true, PlayerOrigin, PlayerExtent);
	FBox PlayerBox = FBox::BuildAABB(PlayerOrigin, PlayerExtent);

	if (UWorld* World = GetWorld())
	{
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor->GetClass()->ImplementsInterface(UEstPlayerSpawn::StaticClass()))
			{
				IEstPlayerSpawn::Execute_OnPlayerSpawn(Actor, NewPlayer, NewCharacter);

				FVector ActorOrigin;
				FVector ActorExtent;
				Actor->GetActorBounds(true, ActorOrigin, ActorExtent);
				FBox ActorBox = FBox::BuildAABB(ActorOrigin, ActorExtent);

				if (ActorBox.Intersect(PlayerBox) || PlayerBox.IsInside(ActorBox))
				{
					IEstPlayerSpawn::Execute_OnPlayerSpawnInside(Actor, NewPlayer, NewCharacter);
				}
			}
		}
	}
}
