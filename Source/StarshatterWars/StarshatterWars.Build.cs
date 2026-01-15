// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class StarshatterWars : ModuleRules
{
    public StarshatterWars(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "Slate",
            "SlateCore",
            "UMG",
            "RenderCore",
            "RHI",
            "MediaAssets",
            "CommonUI"
        });

        // Since you want to include from Systems/ and Planners/ freely:
        PublicIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDirectory, "System"),
            Path.Combine(ModuleDirectory, "Planners"),
            Path.Combine(ModuleDirectory, "Space"),
            Path.Combine(ModuleDirectory, "Screen"),
            Path.Combine(ModuleDirectory, "Game"),
            Path.Combine(ModuleDirectory, "Levels"),
            Path.Combine(ModuleDirectory, "Actors"),
            Path.Combine(ModuleDirectory, "Campaign"),
            Path.Combine(ModuleDirectory, "Foundation"),
        });
    }
}

