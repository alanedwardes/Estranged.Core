#pragma once

#include "EstGameState.generated.h"

USTRUCT(BlueprintType)
struct FEstComponentState
{	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<uint8> ComponentData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform ComponentTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* ComponentClass;
};

USTRUCT(BlueprintType)
struct FEstActorState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<uint8> ActorData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEstComponentState> ComponentStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SaveId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform ActorTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ActorTags;
};

USTRUCT(BlueprintType)
struct FEstSequenceState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrameNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlaying;
};

USTRUCT(BlueprintType)
struct FEstLevelState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEstActorState> ActorStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEstSequenceState> SequenceStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LevelName;
};

USTRUCT(BlueprintType)
struct FEstWorldState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FEstLevelState PersistentLevelState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEstLevelState> StreamingLevelStates;
};