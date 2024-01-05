// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EstLoggerWidget.generated.h"

UENUM(BlueprintType)
enum class EEstLoggerLevel : uint8
{
	Trace,
	Normal,
	Warning,
	Error
};

USTRUCT(BlueprintType)
struct ESTCORE_API FEstLoggerMessage
{
	GENERATED_BODY()

public:
	FEstLoggerMessage() : FEstLoggerMessage(nullptr, EEstLoggerLevel::Normal, FString())
	{
	}

	FEstLoggerMessage(UObject* InSource, EEstLoggerLevel InLevel, FString InText)
	{
		Source = InSource;
		Level = InLevel;
		Text = InText;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TWeakObjectPtr<UObject> Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	EEstLoggerLevel Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString Text;
};

UCLASS(abstract)
class ESTCORE_API UEstLoggerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnLogMessage(FEstLoggerMessage Message);
};
