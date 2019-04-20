#include "EstCore.h"
#include "Modules/ModuleManager.h"

#if PLATFORM_WINDOWS
	THIRD_PARTY_INCLUDES_START
		#include <SDL.h>
	THIRD_PARTY_INCLUDES_END
#endif

extern ENGINE_API float GAverageFPS;

IMPLEMENT_GAME_MODULE(FEstCoreModule, EstCore);

// Default to something low so we don't start at zero
float FEstCoreModule::LongAverageFrameRate = 25.f;

void FEstCoreModule::StartupModule()
{
#if PLATFORM_WINDOWS
	SDL_Init(0);
	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
#endif

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
#if PLATFORM_WINDOWS
	SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
	SDL_Quit();
#endif

	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

DEFINE_LOG_CATEGORY(LogEstGeneral);
DEFINE_LOG_CATEGORY(LogEstFootsteps);

ESTCORE_API const FEstImpactEffect FEstImpactEffect::None = FEstImpactEffect();