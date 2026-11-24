using UnrealBuildTool;

public class DynamicVRTemplateEditorTarget : TargetRules
{
	public DynamicVRTemplateEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("DynamicVRTemplate");
		ExtraModuleNames.Add("DynamicVRTemplateEditorTools");
	}
}
