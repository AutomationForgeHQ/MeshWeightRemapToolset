using UnrealBuildTool;

public class MeshWeightRemapToolset : ModuleRules
{
	public MeshWeightRemapToolset(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"ToolsetRegistry",   // UToolsetDefinition and UAgentSkill are public base classes here
				"MeshWeightRemap",   // the capability this adapter exposes
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",          // GEditor, to reach the editor subsystem
			}
			);
	}
}
