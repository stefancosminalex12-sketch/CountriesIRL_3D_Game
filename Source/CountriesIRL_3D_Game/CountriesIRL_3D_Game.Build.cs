// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CountriesIRL_3D_Game : ModuleRules
{
	public CountriesIRL_3D_Game(ReadOnlyTargetRules Target) : base(Target)
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
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "SlateCore", "EngineSettings" });

		PublicIncludePaths.AddRange(new string[] {
			"CountriesIRL_3D_Game",
			"CountriesIRL_3D_Game/Variant_Platforming",
			"CountriesIRL_3D_Game/Variant_Platforming/Animation",
			"CountriesIRL_3D_Game/Variant_Combat",
			"CountriesIRL_3D_Game/Variant_Combat/AI",
			"CountriesIRL_3D_Game/Variant_Combat/Animation",
			"CountriesIRL_3D_Game/Variant_Combat/Gameplay",
			"CountriesIRL_3D_Game/Variant_Combat/Interfaces",
			"CountriesIRL_3D_Game/Variant_Combat/UI",
			"CountriesIRL_3D_Game/Variant_SideScrolling",
			"CountriesIRL_3D_Game/Variant_SideScrolling/AI",
			"CountriesIRL_3D_Game/Variant_SideScrolling/Gameplay",
			"CountriesIRL_3D_Game/Variant_SideScrolling/Interfaces",
			"CountriesIRL_3D_Game/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
