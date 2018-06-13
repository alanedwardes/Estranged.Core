#pragma once

#include "Engine.h"
#include "EstGameplayStatics.h"
#include "EstConstants.h"
#include "Containers/Ticker.h"

class FEstCoreModule : public IModuleInterface
{
public:
	virtual void StartupModule();
	virtual void ShutdownModule();
	static float GetLongAverageFrameRate() { return LongAverageFrameRate; };
private:
	virtual bool Tick(float DeltaTime);
	static float LongAverageFrameRate;
	FTickerDelegate TickDelegate;
	FDelegateHandle TickDelegateHandle;
};

DECLARE_LOG_CATEGORY_EXTERN(LogEstGeneral, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogEstFootsteps, Log, All);

#define EST_DEBUG(DebugText) if (GEngine->GameViewport->bIsPlayInEditorViewport) GEngine->AddOnScreenDebugMessage(-1, DEBUG_PERSIST_TIME, DEBUG_COLOR, DebugText)
#define EST_DEBUG_SLOT(Slot, DebugText) if (GEngine->GameViewport->bIsPlayInEditorViewport) GEngine->AddOnScreenDebugMessage(Slot, 0.f, DEBUG_COLOR, DebugText)

#define EST_WARNING(WarningText) if (GEngine->GameViewport->bIsPlayInEditorViewport) GEngine->AddOnScreenDebugMessage(-1, WARNING_PERSIST_TIME, WARNING_COLOR, WarningText)
#define EST_WARNING_SLOT(Slot, WarningText) if (GEngine->GameViewport->bIsPlayInEditorViewport) GEngine->AddOnScreenDebugMessage(Slot, 0.f, WARNING_COLOR, WarningText)
