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
#include "Components/DecalComponent.h"
#include "Misc/DataValidation.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EstStaticWidgetRenderer)

// Sets default values
AEstStaticWidgetRenderer::AEstStaticWidgetRenderer(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	MaterialTextureParameterName = TEXT("Texture");
	CanvasRenderTarget2DClass = UCanvasRenderTarget2D::StaticClass();

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

	if (WidgetClass.Get() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEstStaticWidgetRenderer::BeginPlay: No WidgetClass specified on %s"), *GetName());
		return;
	}

	if (CanvasRenderTarget2DClass.Get() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEstStaticWidgetRenderer::BeginPlay: No CanvasRenderTarget2DClass specified on %s"), *GetName());
		return;
	}
	
	if (Material == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEstStaticWidgetRenderer::BeginPlay: No Material specified on %s"), *GetName());
		return;
	}

	WidgetInstance = CreateWidget(GetWorld(), WidgetClass);
	if (WidgetInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEstStaticWidgetRenderer::BeginPlay: CreateWidget failed on %s"), *GetName());
		return;
	}
	SlateWidgetInstance = WidgetInstance->TakeWidget();

	RenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, CanvasRenderTarget2DClass, FMath::RoundToInt(WidgetSize.X), FMath::RoundToInt(WidgetSize.Y));
	if (RenderTarget == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEstStaticWidgetRenderer::BeginPlay: CreateCanvasRenderTarget2D failed on %s"), *GetName());
		return;
	}
	RenderTarget->Rename(*FString::Printf(TEXT("RT_%s_%s"), *GetName(), *WidgetInstance->GetName()), this);
	RenderTarget->UpdateResource();

	DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
	if (DynamicMaterial == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEstStaticWidgetRenderer::BeginPlay: CreateMaterialInstanceDynamic failed on %s"), *GetName());
		return;
	}
	DynamicMaterial->SetTextureParameterValue(MaterialTextureParameterName, RenderTarget);

	WidgetRenderer = new FWidgetRenderer(true);

	for (AActor* Actor : Targets)
	{
		if (Actor == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("AEstStaticWidgetRenderer::BeginPlay: Null Target Actor specified on %s"), *GetName());
			continue;
		}

		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
		if (Primitive != nullptr)
		{	
			Primitive->SetMaterial(MaterialSlot, DynamicMaterial);
		}

		UDecalComponent* Decal = Cast<UDecalComponent>(Actor->GetRootComponent());
		if (Decal != nullptr)
		{
			Decal->SetMaterial(MaterialSlot, DynamicMaterial);
		}
	}

	RenderWidget();
}

void AEstStaticWidgetRenderer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	SlateWidgetInstance = nullptr;
	delete WidgetRenderer;
}

#if WITH_EDITOR
EDataValidationResult AEstStaticWidgetRenderer::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (WidgetClass.Get() == nullptr)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("%s: WidgetClass is not set"), *GetName())));
		Result = EDataValidationResult::Invalid;
	}

	if (Material == nullptr)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("%s: Material is not set"), *GetName())));
		Result = EDataValidationResult::Invalid;
	}

	if (WidgetSize.IsZero())
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("%s: WidgetSize is zero"), *GetName())));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

void AEstStaticWidgetRenderer::RenderWidget()
{
	if (WidgetInstance == nullptr)
	{
		return;
	}

	if (RenderTarget == nullptr)
	{
		return;
	}

	// First render once to force the layout to happen
	WidgetRenderer->DrawWidget(RenderTarget, SlateWidgetInstance.ToSharedRef(), WidgetSize, 0.f, false);
	// Then take the result
	WidgetRenderer->DrawWidget(RenderTarget, SlateWidgetInstance.ToSharedRef(), WidgetSize, 0.f, false);

	RenderTarget->UpdateResourceImmediate(false);

}

void AEstStaticWidgetRenderer::Tick(float DeltaTime)
{
	if (WidgetInstance == nullptr)
	{
		return;
	}

	if (RenderTarget == nullptr)
	{
		return;
	}

	WidgetRenderer->DrawWidget(RenderTarget, SlateWidgetInstance.ToSharedRef(), WidgetSize, 0.f, false);
	RenderTarget->UpdateResourceImmediate(false);
}
