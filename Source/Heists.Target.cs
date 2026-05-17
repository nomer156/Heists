// Copyright 2026 Heists. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HeistsTarget : TargetRules
{
	public HeistsTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("Heists");
	}
}
