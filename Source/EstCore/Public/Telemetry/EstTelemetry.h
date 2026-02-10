#pragma once

#include "CoreMinimal.h"

class ESTCORE_API FEstTelemetry
{
public:
	static void Init();
	static void Shutdown();

private:
	static void OnCrash();
	static void OnStartup();
	static void OnPreExit();
	static void OnOutOfMemory();
	static void OnGPUOutOfMemory(const uint64, const uint64);

	static void OnEndFrame();
	static void OnPreLoadMap(const FString& MapName);
	static void OnPostLoadMap(UWorld* World);

	static void SendReport(const FString& Reason, const FString& QueryParams = TEXT(""));

	static FString CollectGameUserSettings();
};
