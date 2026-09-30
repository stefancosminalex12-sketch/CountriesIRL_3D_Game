// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CrownsAndCommoners : ModuleRules
{
	public CrownsAndCommoners(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"DeveloperSettings",
			"PhysicsCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "SlateCore", "EngineSettings" });

		PublicIncludePaths.AddRange(new string[] {
			"CrownsAndCommoners",
			"CrownsAndCommoners/Variant_Platforming",
			"CrownsAndCommoners/Variant_Platforming/Animation",
			"CrownsAndCommoners/Variant_Combat",
			"CrownsAndCommoners/Variant_Combat/AI",
			"CrownsAndCommoners/Variant_Combat/Animation",
			"CrownsAndCommoners/Variant_Combat/Gameplay",
			"CrownsAndCommoners/Variant_Combat/Interfaces",
			"CrownsAndCommoners/Variant_Combat/UI",
			"CrownsAndCommoners/Variant_SideScrolling",
			"CrownsAndCommoners/Variant_SideScrolling/AI",
			"CrownsAndCommoners/Variant_SideScrolling/Gameplay",
			"CrownsAndCommoners/Variant_SideScrolling/Interfaces",
			"CrownsAndCommoners/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
