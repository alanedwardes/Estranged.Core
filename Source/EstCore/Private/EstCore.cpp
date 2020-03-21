#include "EstCore.h"
#include "Modules/ModuleManager.h"

extern ENGINE_API float GAverageFPS;

IMPLEMENT_GAME_MODULE(FEstCoreModule, EstCore);

// Default to something low so we don't start at zero
float FEstCoreModule::LongAverageFrameRate = 25.f;

void FEstCoreModule::StartupModule()
{
	TickDelegate = FTickerDelegate::CreateRaw(this, &FEstCoreModule::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
}

bool FEstCoreModule::Tick(float DeltaTime)
{
	LongAverageFrameRate = LongAverageFrameRate * .99f + GAverageFPS * .01f;
	return true;
}

void FEstCoreModule::ShutdownModule()
{
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

DEFINE_LOG_CATEGORY(LogEstGeneral);

ESTCORE_API const FEstImpactEffect FEstImpactEffect::None = FEstImpactEffect();