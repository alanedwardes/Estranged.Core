#pragma once

#include "Modules/ModuleInterface.h"
#include "Gameplay/EstGameplayStatics.h"
#include "EstConstants.h"

class ESTCORE_API FEstCoreModule : public IModuleInterface
{
public:
	virtual void StartupModule();
	virtual void ShutdownModule();
};

DECLARE_LOG_CATEGORY_EXTERN(LogEstGeneral, Log, All);