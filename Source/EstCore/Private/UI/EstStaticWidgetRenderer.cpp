// Estranged is a trade mark of Alan Edwardes.

#include "UI/EstStaticWidgetRenderer.h"
#include "Components/BillboardComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "SceneManagement.h"
#include "Kismet/GameplayStatics.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Blueprint/UserWidget.h"

// Sets default values
AEstStaticWidgetRenderer::AEstStaticWidgetRenderer(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// See https://github.com/EpicGames/UnrealEngine/blob/f8f4b403eb682ffc055613c7caf9d2ba5df7f319/Engine/Source/Runtime/Engine/Private/Components/ReflectionCaptureComponent.cpp#L123
#if WITH_EDITORONLY_DATA 
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (!IsRunningCommandlet() && (SpriteComponent != nullptr))
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			FName NAME_StaticWidgetRenderer;
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> DecalTexture;
			FConstructorStatics()
				: NAME_StaticWidgetRenderer(TEXT("StaticWidgetRenderer"))
				, DecalTexture(TEXT("/Engine/EditorResources/S_SceneCaptureIcon"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		SpriteComponent->Sprite = ConstructorStatics.DecalTexture.Get();
		SpriteComponent->SetRelativeScale3D_Direct(FVector(0.5f, 0.5f, 0.5f));
		SpriteComponent->bHiddenInGame = true;
		SpriteComponent->SetUsingAbsoluteScale(true);
		SpriteComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		SpriteComponent->bIsScreenSizeScaled = true;
	}
#endif
}

// Called when the game starts or when spawned
void AEstStaticWidgetRenderer::BeginPlay()
{
	Super::BeginPlay();
	
	WidgetInstance = CreateWidget(GetWorld(), WidgetClass);
	RenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, CanvasRenderTarget2DClass, FMath::RoundToInt(WidgetSize.X), FMath::RoundToInt(WidgetSize.Y));

	DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
	DynamicMaterial->SetTextureParameterValue(MaterialTextureParameterName, RenderTarget);

	WidgetRenderer = new FWidgetRenderer(true);

	for (AActor* Actor : Targets)
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
		if (Primitive != nullptr)
		{	
			Primitive->SetMaterial(MaterialSlot, DynamicMaterial);
		}
	}

	RenderWidget();
}

void AEstStaticWidgetRenderer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	delete WidgetRenderer;
}

void AEstStaticWidgetRenderer::RenderWidget()
{
	TSharedPtr<SWidget> SlateWidget(WidgetInstance->TakeWidget());

	// First render once to force the layout to happen
	WidgetRenderer->DrawWidget(RenderTarget, SlateWidget.ToSharedRef(), WidgetSize, 0.f, false);
	// Then take the result
	WidgetRenderer->DrawWidget(RenderTarget, SlateWidget.ToSharedRef(), WidgetSize, 0.f, false);

	RenderTarget->UpdateResourceImmediate(false);

}