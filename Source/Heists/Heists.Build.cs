// Copyright 2026 Heists. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Heists : ModuleRules
{
	public Heists(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Добавляем корень модуля в include paths — чтобы #include "Character/X.h" работал из любой подпапки
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"NavigationSystem",
			"AIModule",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"NetCore",
			"PhysicsCore",
			"Niagara",
			"UMG",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
		});

		// UE 5.2+ IWYU support
		IWYUSupport = IWYUSupport.Full;
	}
}
