// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SaytEditor : ModuleRules
{
	public SaytEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UnrealEd",
			"UMG",
			"UMGEditor",
			"Blutility",
			"AssetRegistry"
		});
	}
}