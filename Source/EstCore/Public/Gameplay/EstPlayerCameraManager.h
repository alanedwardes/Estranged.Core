// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "EstPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class ESTCORE_API AEstPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	virtual void OnPhotographySessionStart_Implementation() override;
	virtual void OnPhotographySessionEnd_Implementation() override;

private:
	virtual void SetPlayerHidden(bool bIsHidden);
	virtual void SetViewportHidden(bool bIsHidden);
};
