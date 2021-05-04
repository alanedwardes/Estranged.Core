// Estranged is a trade mark of Alan Edwardes.

#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Volumes/EstResetVolume.h"

AEstResetVolume::AEstResetVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetBrushComponent()->SetCollisionProfileName(PROFILE_TRIGGER);
}

void AEstResetVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (TargetsSet.Num() == 0 || Pawn == nullptr)
	{
		return;
	}

	Pawn->GetMovementComponent()->Velocity = FVector::ZeroVector;

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	CameraManager->StartCameraFade(1, 0, 2.f, FLinearColor::Black);

	const TArray<AActor*> TargetsArray = UEstGameplayStatics::SortActorArrayByClosest(OtherActor, TargetsSet.Array());

	const AActor* Target = TargetsArray[0];

	if (!OtherActor->TeleportTo(Target->GetActorLocation(), Target->GetActorRotation()))
	{
		// This shouldn't happen, but we need to force it
		OtherActor->SetActorLocationAndRotation(Target->GetActorLocation(), Target->GetActorRotation());
	}
}
