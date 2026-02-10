#include "Telemetry/EstTelemetry.h"
#include "EstCore.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/ScopeLock.h"
#include "GameFramework/GameUserSettings.h"
#include "Async/Async.h"
#include "Misc/App.h"
#include "Engine/World.h"

static FDelegateHandle OnCrashHandle;
static FDelegateHandle OnStartupHandle;
static FDelegateHandle OnOutOfMemoryHandle;
static FDelegateHandle OnGPUOutOfMemoryHandle;
static FDelegateHandle OnPreExitHandle;

static FDelegateHandle OnEndFrameHandle;
static FDelegateHandle PreLoadMapHandle;
static FDelegateHandle PostLoadMapHandle;

static bool bIsLoading = false;
static double TotalSessionTime = 0.0;
static uint64 TotalFrameCount = 0;
static uint64 Frames_Above60 = 0;
static uint64 Frames_30to60 = 0;
static uint64 Frames_Below30 = 0;

void FEstTelemetry::Init()
{
	OnCrashHandle = FCoreDelegates::OnHandleSystemError.AddStatic(&FEstTelemetry::OnCrash);
	OnStartupHandle = FCoreDelegates::OnFEngineLoopInitComplete.AddStatic(&FEstTelemetry::OnStartup);
	OnOutOfMemoryHandle = FCoreDelegates::GetOutOfMemoryDelegate().AddStatic(&FEstTelemetry::OnOutOfMemory);
	OnGPUOutOfMemoryHandle = FCoreDelegates::GetGPUOutOfMemoryDelegate().AddStatic(&FEstTelemetry::OnGPUOutOfMemory);
	OnPreExitHandle = FCoreDelegates::OnPreExit.AddStatic(&FEstTelemetry::OnPreExit);

	OnEndFrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&FEstTelemetry::OnEndFrame);
	PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddStatic(&FEstTelemetry::OnPreLoadMap);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddStatic(&FEstTelemetry::OnPostLoadMap);

	// Reset stats
	TotalSessionTime = 0.0;
	TotalFrameCount = 0;
	Frames_Above60 = 0;
	Frames_30to60 = 0;
	Frames_Below30 = 0;
	bIsLoading = false;
}

void FEstTelemetry::Shutdown()
{
	FCoreDelegates::OnHandleSystemError.Remove(OnCrashHandle);
	FCoreDelegates::OnFEngineLoopInitComplete.Remove(OnStartupHandle);
	FCoreDelegates::GetOutOfMemoryDelegate().Remove(OnOutOfMemoryHandle);
	FCoreDelegates::GetGPUOutOfMemoryDelegate().Remove(OnGPUOutOfMemoryHandle);
	FCoreDelegates::OnPreExit.Remove(OnPreExitHandle);
	
	OnCrashHandle.Reset();
	OnStartupHandle.Reset();
	OnOutOfMemoryHandle.Reset();
	OnGPUOutOfMemoryHandle.Reset();
	OnPreExitHandle.Reset();

	FCoreDelegates::OnEndFrame.Remove(OnEndFrameHandle);
	FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);

	OnEndFrameHandle.Reset();
	PreLoadMapHandle.Reset();
	PostLoadMapHandle.Reset();
}

void FEstTelemetry::OnCrash()
{
	SendReport(TEXT("crash"));
}

void FEstTelemetry::OnStartup()
{
	SendReport(TEXT("startup"));
}

void FEstTelemetry::OnPreExit()
{
	if (TotalFrameCount > 0)
	{
		FString Params;
		
		const double AvgFps = TotalSessionTime > 0 ? (double)TotalFrameCount / TotalSessionTime : 0.0;
		Params += FString::Printf(TEXT("avg_fps=%.2f&"), AvgFps);
		
		const float PctAbove60 = (float)Frames_Above60 / (float)TotalFrameCount * 100.0f;
		Params += FString::Printf(TEXT("pct_above_60=%.2f&"), PctAbove60);
		
		const float Pct30to60 = (float)Frames_30to60 / (float)TotalFrameCount * 100.0f;
		Params += FString::Printf(TEXT("pct_30_to_60=%.2f&"), Pct30to60);
		
		const float PctBelow30 = (float)Frames_Below30 / (float)TotalFrameCount * 100.0f;
		Params += FString::Printf(TEXT("pct_below_30=%.2f&"), PctBelow30);

		Params += FString::Printf(TEXT("session_time=%.2f"), TotalSessionTime);

		SendReport(TEXT("frames"), Params);
	}
	
	SendReport(TEXT("settings"), CollectGameUserSettings());
}

void FEstTelemetry::OnOutOfMemory()
{
	SendReport(TEXT("oom"));
}

void FEstTelemetry::OnGPUOutOfMemory(const uint64 Size, const uint64 Available)
{
	SendReport(TEXT("oom_gpu"), FString::Printf(TEXT("size=%llu&available=%llu"), Size, Available));
}

void FEstTelemetry::OnEndFrame()
{
	if (bIsLoading)
	{
		return;
	}

	if (GWorld == nullptr || !GWorld->HasBegunPlay())
	{
		return;
	}

	const double DT = FApp::GetDeltaTime();
	if (DT <= 0.0)
	{
		return;
	}

	TotalSessionTime += DT;
	TotalFrameCount++;

	if (DT <= 0.01667) // 60 FPS = 16.666ms
	{
		Frames_Above60++;
	}
	else if (DT <= 0.03334) // 30 FPS = 33.333ms
	{
		Frames_30to60++;
	}
	else
	{
		Frames_Below30++;
	}
}

void FEstTelemetry::OnPreLoadMap(const FString& MapName)
{
	bIsLoading = true;
}

void FEstTelemetry::OnPostLoadMap(UWorld* World)
{
	bIsLoading = false;
}

void FEstTelemetry::SendReport(const FString& Reason, const FString& QueryParams)
{
	FString StatsEndpoint;
	if (GConfig)
	{
		GConfig->GetString(TEXT("/Script/EstCore.EstCore"), TEXT("StatsEndpoint"), StatsEndpoint, GEngineIni);
	}

	if (StatsEndpoint.IsEmpty())
	{
		return;
	}

	if (!StatsEndpoint.EndsWith(TEXT("/")))
	{
		StatsEndpoint += TEXT("/");
	}
	StatsEndpoint += Reason;

	if (!QueryParams.IsEmpty())
	{
		StatsEndpoint += TEXT("?") + QueryParams;
	}

	FHttpModule* Http = &FHttpModule::Get();
	if (Http == nullptr)
	{
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetVerb(TEXT("GET"));
	Request->SetURL(StatsEndpoint);
	Request->SetTimeout(2.0f);
	Request->SetHeader(TEXT("Cache-Control"), TEXT("no-cache"));
	Request->SetHeader(TEXT("Pragma"), TEXT("no-cache"));
	Request->ProcessRequest();
}

FString FEstTelemetry::CollectGameUserSettings()
{
	if (GEngine == nullptr)
	{
		return FString();
	}

	UGameUserSettings* GUS = UGameUserSettings::GetGameUserSettings();
	if (GUS == nullptr)
	{
		return FString();
	}

	int32 sg_Res = (int32)GUS->ScalabilityQuality.ResolutionQuality;
	int32 sg_View = GUS->ScalabilityQuality.ViewDistanceQuality;
	int32 sg_AA = GUS->ScalabilityQuality.AntiAliasingQuality;
	int32 sg_Shadow = GUS->ScalabilityQuality.ShadowQuality;
	int32 sg_GI = GUS->ScalabilityQuality.GlobalIlluminationQuality;
	int32 sg_Refl = GUS->ScalabilityQuality.ReflectionQuality;
	int32 sg_PP = GUS->ScalabilityQuality.PostProcessQuality;
	int32 sg_Tex = GUS->ScalabilityQuality.TextureQuality;
	int32 sg_FX = GUS->ScalabilityQuality.EffectsQuality;
	int32 sg_Foliage = GUS->ScalabilityQuality.FoliageQuality;
	int32 sg_Shading = GUS->ScalabilityQuality.ShadingQuality;
	int32 sg_Landscape = GUS->ScalabilityQuality.LandscapeQuality;

	FString Params;
	Params += FString::Printf(TEXT("sg.ResolutionQuality=%d&"), sg_Res);
	Params += FString::Printf(TEXT("sg.ViewDistanceQuality=%d&"), sg_View);
	Params += FString::Printf(TEXT("sg.AntiAliasingQuality=%d&"), sg_AA);
	Params += FString::Printf(TEXT("sg.ShadowQuality=%d&"), sg_Shadow);
	Params += FString::Printf(TEXT("sg.GlobalIlluminationQuality=%d&"), sg_GI);
	Params += FString::Printf(TEXT("sg.ReflectionQuality=%d&"), sg_Refl);
	Params += FString::Printf(TEXT("sg.PostProcessQuality=%d&"), sg_PP);
	Params += FString::Printf(TEXT("sg.TextureQuality=%d&"), sg_Tex);
	Params += FString::Printf(TEXT("sg.EffectsQuality=%d&"), sg_FX);
	Params += FString::Printf(TEXT("sg.FoliageQuality=%d&"), sg_Foliage);
	Params += FString::Printf(TEXT("sg.ShadingQuality=%d&"), sg_Shading);
	Params += FString::Printf(TEXT("sg.LandscapeQuality=%d&"), sg_Landscape);
	
	bool bUseVSync = GUS->IsVSyncEnabled();
	FIntPoint Res = GUS->GetScreenResolution();
	int32 FullscreenMode = (int32)GUS->GetFullscreenMode();
	float FrameRateLimit = GUS->GetFrameRateLimit();	
	bool bUseHDR = GUS->IsHDREnabled();
	int32 HDRNits = GUS->GetCurrentHDRDisplayNits();

	Params += FString::Printf(TEXT("bUseVSync=%s&"), bUseVSync ? TEXT("True") : TEXT("False"));
	Params += FString::Printf(TEXT("ResolutionSizeX=%d&"), Res.X);
	Params += FString::Printf(TEXT("ResolutionSizeY=%d&"), Res.Y);
	Params += FString::Printf(TEXT("FullscreenMode=%d&"), FullscreenMode);
	Params += FString::Printf(TEXT("FrameRateLimit=%f&"), FrameRateLimit);
	Params += FString::Printf(TEXT("bUseHDRDisplayOutput=%s&"), bUseHDR ? TEXT("True") : TEXT("False"));
	Params += FString::Printf(TEXT("HDRDisplayOutputNits=%d"), HDRNits);

	return Params;
}
