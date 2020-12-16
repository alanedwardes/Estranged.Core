// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/EstInteractive.h"
#include "Interfaces/EstSaveRestore.h"
#include "EstDoor.generated.h"

UENUM(BlueprintType)
enum EEstDoorState
{
	Closed,
	Opened,
	Opening UMETA(Hidden),
	Closing UMETA(Hidden)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStateChangeSignature, AEstDoor*, Door);

UCLASS(BlueprintType)
class ESTCORE_API AEstDoor : public AActor, public IEstInteractive, public IEstSaveRestore
{
	GENERATED_BODY()
	
public:	
	AEstDoor();

protected:
	virtual void BeginPlay() override;

	virtual void SetDoorState(EEstDoorState NewDoorState);

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure)
	virtual EEstDoorState GetDoorState() { return DoorState; };

	UFUNCTION(BlueprintCallable)
	virtual bool TrySetDoorState(class AEstBaseCharacter* User, EEstDoorState NewDoorState);

	UFUNCTION(BlueprintCallable)
	virtual void Open(class AEstBaseCharacter* User) { TrySetDoorState(User, EEstDoorState::Opening); };

	UFUNCTION(BlueprintCallable)
	virtual void Close(class AEstBaseCharacter* User) { TrySetDoorState(User, EEstDoorState::Closing); };

	UFUNCTION(BlueprintCallable)
	virtual void Lock() { bDoorIsLocked = true; };

	UFUNCTION(BlueprintCallable)
	virtual void Unlock() { bDoorIsLocked = false; };

	UFUNCTION(BlueprintPure)
	virtual bool IsLocked() { return bDoorIsLocked; };

	UPROPERTY(BlueprintAssignable)
	FStateChangeSignature OnOpening;

	UPROPERTY(BlueprintAssignable)
	FStateChangeSignature OnOpened;

	UPROPERTY(BlueprintAssignable)
	FStateChangeSignature OnClosing;

	UPROPERTY(BlueprintAssignable)
	FStateChangeSignature OnClosed;

// Begin IEstInteractive
	bool OnUsed_Implementation(class AEstBaseCharacter* User, class USceneComponent* UsedComponent) override;
	void OnUnused_Implementation(class AEstBaseCharacter* User) override {};
// End IEstInteractive

// Begin IEstSaveRestore
	void OnPreRestore_Implementation() override {};
	void OnPostRestore_Implementation() override;
	void OnPreSave_Implementation() override;
	void OnPostSave_Implementation() override {};
// End IEstSaveRestore

// Begin SaveId
	virtual FGuid GetSaveId_Implementation() override { return SaveId; };
	virtual void OnConstruction(const FTransform &Transform) override { UEstGameplayStatics::NewGuidIfInvalid(SaveId); };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = SaveState)
	FGuid SaveId;
// End SaveId

	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere)
	TEnumAsByte<EEstDoorState> DoorState;

	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DoorOpenAmount;

	UPROPERTY(SaveGame, BlueprintReadOnly, EditAnywhere)
	bool bDoorIsLocked;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};
