#include "Telemetry/EstTelemetry.h"
#include "EstCore.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/ScopeLock.h"

static FDelegateHandle OnCrashHandle;
static FDelegateHandle OnStartupHandle;
static FDelegateHandle OnOutOfMemoryHandle;
static FDelegateHandle OnGPUOutOfMemoryHandle;

void FEstTelemetry::Init()
{
	OnCrashHandle = FCoreDelegates::OnHandleSystemError.AddStatic(&FEstTelemetry::OnCrash);
	OnStartupHandle = FCoreDelegates::OnFEngineLoopInitComplete.AddStatic(&FEstTelemetry::OnStartup);
	OnOutOfMemoryHandle = FCoreDelegates::GetOutOfMemoryDelegate().AddStatic(&FEstTelemetry::OnOutOfMemory);
	OnGPUOutOfMemoryHandle = FCoreDelegates::GetGPUOutOfMemoryDelegate().AddStatic(&FEstTelemetry::OnGPUOutOfMemory);
}

void FEstTelemetry::Shutdown()
{
	FCoreDelegates::OnHandleSystemError.Remove(OnCrashHandle);
	FCoreDelegates::OnFEngineLoopInitComplete.Remove(OnStartupHandle);
	FCoreDelegates::GetOutOfMemoryDelegate().Remove(OnOutOfMemoryHandle);
	FCoreDelegates::GetGPUOutOfMemoryDelegate().Remove(OnGPUOutOfMemoryHandle);
	
	OnCrashHandle.Reset();
	OnStartupHandle.Reset();
	OnOutOfMemoryHandle.Reset();
	OnGPUOutOfMemoryHandle.Reset();
}

void FEstTelemetry::OnCrash()
{
	SendReport(TEXT("crash"));
}

void FEstTelemetry::OnStartup()
{
	SendReport(TEXT("startup"));
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
