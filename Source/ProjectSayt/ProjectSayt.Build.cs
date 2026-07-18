// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectSayt : ModuleRules
{
	public ProjectSayt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.Add("ProjectSayt");
	
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
			// UnrealEd 제거됨 — 에디터 전용 코드는 SayuEditor 모듈로 분리 완료 (Phase 7)
		});

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
