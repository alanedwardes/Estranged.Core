#pragma once

UENUM(BlueprintType)
enum class EEstLoggerLevel : uint8
{
	Trace,
	Normal,
	Warning,
	Error
};

void GEstLogMessage(UObject* Source, EEstLoggerLevel Level, FString Message);

#define EST_LOG(Source, Level, Format, ...) \
	GEstLogMessage(Source, EEstLoggerLevel::Level, FString::Printf(TEXT(Format), ##__VA_ARGS__))