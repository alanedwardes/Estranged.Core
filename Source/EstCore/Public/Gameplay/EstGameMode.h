// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EstGameMode.generated.h"

UCLASS()
class ESTCORE_API AEstGameMode : public AGameMode
{
	GENERATED_BODY()

	virtual void HandleMatchHasStarted() override;

public:
	UFUNCTION(BlueprintNativeEvent, Category = State)
	void OnPreBeginPlay();

	void RestartPlayer(AController* NewPlayer) override;

	APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void WorldBeginPlay();

private:
	TMap<class APawn*, class AEstPlayerStart*> StartedPawns;
};