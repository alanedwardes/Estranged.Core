#include "Telemetry/EstTelemetry.h"
#include "EstCore.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/ScopeLock.h"
#include "GameFramework/GameUserSettings.h"
#include "Async/Async.h"

static FDelegateHandle OnCrashHandle;
static FDelegateHandle OnStartupHandle;
static FDelegateHandle OnOutOfMemoryHandle;
static FDelegateHandle OnGPUOutOfMemoryHandle;
static FDelegateHandle OnPreExitHandle;
static FDelegateHandle ConfigSaveHandle;

void FEstTelemetry::Init()
{
	OnCrashHandle = FCoreDelegates::OnHandleSystemError.AddStatic(&FEstTelemetry::OnCrash);
	OnStartupHandle = FCoreDelegates::OnFEngineLoopInitComplete.AddStatic(&FEstTelemetry::OnStartup);
	OnOutOfMemoryHandle = FCoreDelegates::GetOutOfMemoryDelegate().AddStatic(&FEstTelemetry::OnOutOfMemory);
	OnGPUOutOfMemoryHandle = FCoreDelegates::GetGPUOutOfMemoryDelegate().AddStatic(&FEstTelemetry::OnGPUOutOfMemory);
	OnPreExitHandle = FCoreDelegates::OnPreExit.AddStatic(&FEstTelemetry::OnPreExit);
	ConfigSaveHandle = FCoreDelegates::TSPreSaveConfigFileDelegate().AddStatic(&FEstTelemetry::OnConfigSaved);
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

	if (ConfigSaveHandle.IsValid())
	{
		FCoreDelegates::TSPreSaveConfigFileDelegate().Remove(ConfigSaveHandle);
		ConfigSaveHandle.Reset();
	}
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
	SendReport(TEXT("settings"), CollectGameUserSettings());
}

void FEstTelemetry::OnConfigSaved(const TCHAR* IniFilename, const FString& ContentsToSave, int32& SavedCount)
{
	if (FString(IniFilename).Contains(TEXT("GameUserSettings")))
	{
		AsyncTask(ENamedThreads::GameThread, []()
		{
			SendReport(TEXT("settings"), CollectGameUserSettings());
		});
	}
}

void FEstTelemetry::OnOutOfMemory()
{
	SendReport(TEXT("oom"));
}

void FEstTelemetry::OnGPUOutOfMemory(const uint64 Size, const uint64 Available)
{
	SendReport(TEXT("oom_gpu"), FString::Printf(TEXT("size=%llu&available=%llu"), Size, Available));
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
