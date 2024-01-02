#include "EstCore.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_GAME_MODULE(FEstCoreModule, EstCore);

void FEstCoreModule::StartupModule()
{
}

void FEstCoreModule::ShutdownModule()
{
}

DEFINE_LOG_CATEGORY(LogEstGeneral);
DEFINE_LOG_CATEGORY(LogEstPhysicsImpacts);

ESTCORE_API const FEstImpactEffect FEstImpactEffect::None = FEstImpactEffect();