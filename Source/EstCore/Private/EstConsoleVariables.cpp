#include "EstConsoleVariables.h"

TAutoConsoleVariable<bool> FEstConsoleVariables::EnableForceFeedback(
	TEXT("est.EnableForceFeedback"),
	true,
	TEXT("Defines whether force feedback is enabled for connected gamepads."),
	ECVF_Default
);