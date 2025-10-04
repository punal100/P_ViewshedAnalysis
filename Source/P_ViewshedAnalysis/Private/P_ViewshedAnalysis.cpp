/*
 * @Author: Punal Manalan
 * @Description: ViewShed Analysis Plugin.
 * @Date: 04/10/2025
 */

#include "P_ViewshedAnalysis.h"

// Text localization namespace for this module
#define LOCTEXT_NAMESPACE "FP_ViewShedAnalysisModule"

/**
 * Module startup - called after module is loaded into memory
 * Initialize any global resources or register systems here
 */
void FP_ViewShedAnalysisModule::StartupModule()
{
	// Module startup logic goes here
	// Currently empty as we don't need special initialization
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
IMPLEMENT_MODULE(FP_ViewShedAnalysisModule, P_ViewShedAnalysis)