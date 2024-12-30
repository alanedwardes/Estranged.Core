// Estranged is a trade mark of Alan Edwardes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EstStaticWidgetRenderer.generated.h"

UCLASS()
class ESTCORE_API AEstStaticWidgetRenderer : public AActor
{
	GENERATED_BODY()
	
public:	
	AEstStaticWidgetRenderer(const class FObjectInitializer& PCIP);

	UFUNCTION(BlueprintCallable)
	virtual void RenderWidget();

	/** Tick to check if primary/secondary attack is pressed */
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
	class UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
	TArray<AActor*> Targets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
	int32 MaterialSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
	FName MaterialTextureParameterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
	TSubclassOf<class UUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
	TSubclassOf<class UCanvasRenderTarget2D> CanvasRenderTarget2DClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widget)
	FVector2D WidgetSize;

private:

	class FWidgetRenderer* WidgetRenderer;

#if WITH_EDITORONLY_DATA 
	UPROPERTY()
	class UBillboardComponent* SpriteComponent;
#endif

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;

	UPROPERTY()
	class UTextureRenderTarget2D* RenderTarget;

	UPROPERTY()
	class UUserWidget* WidgetInstance;

	TSharedPtr<SWidget> SlateWidgetInstance;
};
