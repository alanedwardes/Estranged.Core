// Estranged is a trade mark of Alan Edwardes.

#include "EstCore.h"
#include "EstPlayer.h"
#include "EstBaseWeapon.h"
#include "EstPlayerCameraManager.h"

void AEstPlayerCameraManager::OnPhotographySessionStart_Implementation()
{
	Super::OnPhotographySessionStart_Implementation();

	SetPlayerHidden(true);
	SetViewportHidden(true);

	UEstGameplayStatics::SetTransientMasterVolume(this, 0.f);
}

void AEstPlayerCameraManager::OnPhotographySessionEnd_Implementation()
{
	Super::OnPhotographySessionStart_Implementation();

	SetPlayerHidden(false);
	SetViewportHidden(false);

	UEstGameplayStatics::SetTransientMasterVolume(this, 1.f);
}

void AEstPlayerCameraManager::SetPlayerHidden(bool bIsHidden)
{
	APlayerController* Controller = GetOwningPlayerController();
	if (Controller == nullptr)
	{
		return;
	}

	AEstPlayer* Player = Cast<AEstPlayer>(Controller->GetPawn());
	if (Player == nullptr)
	{
		return;
	}

	Player->SetActorHiddenInGame(bIsHidden);

	if (Player->HasWeapon())
	{
		Player->EquippedWeapon->SetActorHiddenInGame(bIsHidden);
	}

	if (Player->IsHoldingActor())
	{
		Player->HeldActor->SetActorHiddenInGame(bIsHidden);
	}
}

void AEstPlayerCameraManager::SetViewportHidden(bool bIsHidden)
{
	const EVisibility NewVisibility = bIsHidden ? EVisibility::Hidden : EVisibility::Visible;

	FChildren* ViewportChildren = GEngine->GetGameViewportWidget()->GetChildren();
	for (int32 i = 0; i < ViewportChildren->Num(); i++)
	{
		TSharedRef<SWidget> ViewportChild = ViewportChildren->GetChildAt(i);
		ViewportChild->SetVisibility(NewVisibility);
	}
}
