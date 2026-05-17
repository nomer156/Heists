// Copyright 2026 Heists. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HeistsEditorTarget : TargetRules
{
	public HeistsEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("Heists");
	}
}
