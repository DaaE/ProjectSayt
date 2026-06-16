// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectGAS : ModuleRules
{
	public ProjectGAS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",		// Enhanced Input
			"GameplayAbilities",	// GAS
			"GameplayTags",			// GAS 에서 사용
			"GameplayTasks",		// GAS AbilityTask
			"UMG",					// UI
			"CommonUI",				// Common UI
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",			
			"SlateCore",
			"UnrealEd"				//에디터 툴용 (Editor 모듈 분리 전 임시)
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
