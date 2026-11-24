using UnrealBuildTool;

public class DynamicVRTemplateEditorTools : ModuleRules
{
    public DynamicVRTemplateEditorTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "DynamicVRTemplate", "UnrealEd", "ToolMenus", "Slate", "SlateCore"
        });
    }
}
