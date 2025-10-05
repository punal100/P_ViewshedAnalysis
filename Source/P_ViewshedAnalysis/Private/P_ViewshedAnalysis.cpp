/*
 * @Author: Punal Manalan
 * @Description: ViewShed Analysis Plugin.
 * @Date: 04/10/2025
 */

#include "P_ViewshedAnalysis.h"
// For shader directory mapping and filesystem checks
#include "RenderCore.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

// Text localization namespace for this module
#define LOCTEXT_NAMESPACE "FP_ViewshedAnalysisModule"

/**
 * Module startup - called after module is loaded into memory
 * Initialize any global resources or register systems here
 */
void FP_ViewShedAnalysisModule::StartupModule()
{
	// Map this plugin's Shaders directory so Custom HLSL includes can reference it
	const FString PluginShaderDir = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("P_ViewshedAnalysis/Shaders"));
	if (IFileManager::Get().DirectoryExists(*PluginShaderDir))
	{
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/P_ViewshedAnalysis"), PluginShaderDir);
	}
}

/**
 * Module shutdown - called before module is unloaded
 * Clean up any global resources or unregister systems here
 */
void FP_ViewShedAnalysisModule::ShutdownModule()
{
	// Module cleanup logic goes here
	// Currently empty as we don't have resources to clean up
}

// Undefine the localization namespace to avoid conflicts
#undef LOCTEXT_NAMESPACE

// Register this module with Unreal's module system
IMPLEMENT_MODULE(FP_ViewShedAnalysisModule, P_ViewshedAnalysis)