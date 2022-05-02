#pragma once

#include "Modules/ModuleInterface.h"
#include "Gameplay/EstGameplayStatics.h"
#include "EstConstants.h"
#include "Containers/Ticker.h"

class ESTCORE_API FEstCoreModule : public IModuleInterface
{
public:
	virtual void StartupModule();
	virtual void ShutdownModule();
	static float GetLongAverageFrameRate() { return LongAverageFrameRate; };
private:
	virtual bool Tick(float DeltaTime);
	static float LongAverageFrameRate;
	FTickerDelegate TickDelegate;
	FTSTicker::FDelegateHandle TickDelegateHandle;
};

DECLARE_LOG_CATEGORY_EXTERN(LogEstGeneral, Log, All);