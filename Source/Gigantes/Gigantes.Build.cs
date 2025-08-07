// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gigantes : ModuleRules
{
	public Gigantes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule", "NavigationSystem",
			"GameplayTasks", "GameplayTags", "UMG", "AnimGraphRuntime" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
		
		PublicIncludePaths.AddRange(new string[]
		{
			"Source/Gigantes/Character",
			"Source/Gigantes/GameModes",
			"Source/Gigantes/Player",
			"Source/Gigantes/Items",
			"Source/Gigantes/UI",
			"Source/Gigantes/AI",
			"Source/Gigantes/Weapons",
			"Source/Gigantes/Input",
			"Source/Gigantes/Gameplay",
			"Source/Gigantes/Animation"
		});
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
	
}
