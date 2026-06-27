// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectGAS : ModuleRules
{
	public ProjectGAS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.Add("ProjectGAS");
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",		// Enhanced Input
			"GameplayAbilities",	// GAS
			"GameplayTags",			// use in GAS 
			"GameplayTasks",		// GAS AbilityTask
			"UMG",					// UI
			"CommonUI",				// Common UI
			"Niagara",
			"GameplayMessageRuntime"	// GMS (Phase 6) - 이벤트 버스
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",			
			"SlateCore",
			"UnrealEd"				// for editor tool (Editor 분리 전 임시)
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
