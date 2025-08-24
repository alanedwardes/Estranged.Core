#include "EstLog.h"
#include "Gameplay/EstGameInstance.h"
#include "Gameplay/EstGameplayStatics.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

void GEstLogMessage(UObject* Source, EEstLoggerLevel Level, FString Message)
{
	if (Source == nullptr)
	{
		UE_LOG(LogEstGeneral, Log, TEXT("%s"), *Message);
	}
	else
	{
		UE_LOG(LogEstGeneral, Log, TEXT("%s: %s"), *Source->GetName(), *Message);
	}

	UWorld* World = GEngine->GetCurrentPlayWorld(Source != nullptr ? Source->GetWorld() : nullptr);
	if (World != nullptr)
	{
		UEstGameInstance* GameInstance = World->GetGameInstance<UEstGameInstance>();
		if (GameInstance != nullptr && GameInstance->GetLoggerEnabled())
		{
			GameInstance->LogMessage(FEstLoggerMessage(Source, Level, Message));
		}
	}
}