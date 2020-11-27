#pragma once

#include "EstConstants.h"
#include "GameFramework/PhysicsVolume.h"
#include "Components/BrushComponent.h"
#include "EstCapabilityVolume.generated.h"

UCLASS()
class AEstCapabilityVolume : public APhysicsVolume
{
	GENERATED_BODY()

public:
	AEstCapabilityVolume(const class FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		GetBrushComponent()->SetCollisionProfileName(PROFILE_TRIGGER);

		CanJump = true;
		CanSprint = true;
		CanCrouch = true;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
	bool CanJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
	bool CanSprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
	bool CanCrouch;
};
