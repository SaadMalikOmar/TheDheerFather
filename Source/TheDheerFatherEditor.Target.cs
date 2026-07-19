using UnrealBuildTool;
using System.Collections.Generic;

public class TheDheerFatherEditorTarget : TargetRules
{
	public TheDheerFatherEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("TheDheerFather");
	}
}
