// Estranged is a trade mark of Alan Edwardes.

#include "EstCore.h"
#include "EstPlayer.h"
#include "EstBaseWeapon.h"
#include "EstPlayerCameraManager.h"

void AEstPlayerCameraManager::OnPhotographySessionStart_Implementation()
{
	Super::OnPhotographySessionStart_Implementation();

	SetPlayerHidden(true);
}

void AEstPlayerCameraManager::OnPhotographySessionEnd_Implementation()
{
	Super::OnPhotographySessionStart_Implementation();

	SetPlayerHidden(false);
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