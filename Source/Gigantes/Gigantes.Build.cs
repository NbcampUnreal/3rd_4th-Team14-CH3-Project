// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gigantes : ModuleRules
{
	public Gigantes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"AIModule", 
			"NavigationSystem",
			"GameplayTasks", 
			"GameplayTags", 
			"UMG",
      		"AnimGraphRuntime",
			"Json",
			"JsonUtilities",
			"Niagara",
			"AssetRegistry",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraph", "AITestSuite", "AITestSuite", "Eigen" });
		
		// if (Target.Type == TargetType.Editor)
		// {
		// 	PrivateDependencyModuleNames.Add("UnrealEd");
		// }
		
		PublicIncludePaths.AddRange(new string[]
		{
			"Gigantes",
			"Source/Gigantes/Animation",
			"Source/Gigantes/Character",
			"Source/Gigantes/Enemy",
			"Source/Gigantes/Equipments",
			"Source/Gigantes/GameModes",
			"Source/Gigantes/Gameplay",
			"Source/Gigantes/Input",
			"Source/Gigantes/Items",
			"Source/Gigantes/Physics",
			"Source/Gigantes/Player",
			"Source/Gigantes/UI"
		});
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
	
}
