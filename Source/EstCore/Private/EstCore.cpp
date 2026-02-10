#include "EstCore.h"
#include "Telemetry/EstTelemetry.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_GAME_MODULE(FEstCoreModule, EstCore);

void FEstCoreModule::StartupModule()
{
	FEstTelemetry::Init();
}

void FEstCoreModule::ShutdownModule()
{
	FEstTelemetry::Shutdown();
}

DEFINE_LOG_CATEGORY(LogEstGeneral);

ESTCORE_API const FEstImpactEffect FEstImpactEffect::None = FEstImpactEffect();