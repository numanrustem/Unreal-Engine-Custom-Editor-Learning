// Copyright Epic Games, Inc. All Rights Reserved.



#include "ShaderReader.h"
#include "ShaderCore.h"
#include "Misc/Paths.h"

#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"


#define LOCTEXT_NAMESPACE "FShaderReaderModule"

void FShaderReaderModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FString ShaderDir =  FPaths::Combine(FPaths::ProjectPluginsDir(),TEXT("ShaderReader/Shaders"));	
	AddShaderSourceDirectoryMapping("/ART/Shaders",ShaderDir);
	
}

void FShaderReaderModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FShaderReaderModule, ShaderReader)