// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StarshatterWars : ModuleRules
{
    public StarshatterWars(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "UMG",
            "Core",
            "CoreUObject",
            "Slate",
            "SlateCore",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "MediaAssets" }
        );
    }
}
