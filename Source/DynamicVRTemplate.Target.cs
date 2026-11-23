using UnrealBuildTool;

public class DynamicVRTemplateTarget : TargetRules
{
	public DynamicVRTemplateTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("DynamicVRTemplate");
	}
}
