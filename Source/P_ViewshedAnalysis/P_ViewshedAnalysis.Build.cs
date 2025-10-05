/*
* @Author: Punal Manalan
* @Description: ViewShed Analysis Plugin.
* @Date: 04/10/2025
*/

using UnrealBuildTool;

// Build configuration for the ViewShed Analysis runtime module
public class P_ViewshedAnalysis : ModuleRules
{
	public P_ViewshedAnalysis(ReadOnlyTargetRules Target) : base(Target)
	{
		// Use explicit precompiled headers for better performance
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public include paths - these are exposed to other modules
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);

		// Private include paths - internal to this module only
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);

		// Public dependencies - modules that are required by public headers
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"RenderCore",
				"RHI",
				"ProceduralMeshComponent"
				// ... add other public dependencies that you statically link with here ...
			}
			);

		// Private dependencies - modules only needed for implementation
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);

		// Dynamically loaded modules - loaded at runtime when needed
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
