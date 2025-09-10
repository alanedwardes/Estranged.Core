#pragma once

#include "Runtime/CoreUObject/Public/UObject/Interface.h"
#include "EstLadder.generated.h"

USTRUCT(BlueprintType)
struct ESTCORE_API FLadderExtents
{
	GENERATED_BODY()

	FLadderExtents()
	{
		StartPosition = FVector::ZeroVector;
		EndPosition = FVector::ZeroVector;
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector StartPosition;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector EndPosition;
};

UINTERFACE()
class ESTCORE_API UEstLadder : public UInterface
{
	GENERATED_BODY()
};

class ESTCORE_API IEstLadder
{
	GENERATED_IINTERFACE_BODY()

	/** Get the extents of the ladder (start and end positions) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FLadderExtents GetLadderExtents();
};

