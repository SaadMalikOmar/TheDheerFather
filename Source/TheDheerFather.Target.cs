using UnrealBuildTool;
using System.Collections.Generic;

public class TheDheerFatherTarget : TargetRules
{
	public TheDheerFatherTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("TheDheerFather");
	}
}
