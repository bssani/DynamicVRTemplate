using UnrealBuildTool;

public class DynamicVRTemplate : ModuleRules
{
	public DynamicVRTemplate(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"HeadMountedDisplay", "XRBase", "ChaosVehicles", "OculusXRInput", "VRCalibrationCore"
		});
		PrivateDependencyModuleNames.AddRange(new string[] { "RenderCore", "RHI" });
	}
}
