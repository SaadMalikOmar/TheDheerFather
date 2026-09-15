// Copyright (c) 2026 Saad Malik Omar. All Rights Reserved.
using UnrealBuildTool;

public class TheDheerFather : ModuleRules
{
	public TheDheerFather(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"OnlineSubsystem",
			"OnlineSubsystemNull",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
