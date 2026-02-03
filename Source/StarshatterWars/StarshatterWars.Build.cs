// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class StarshatterWars : ModuleRules
{
    public StarshatterWars(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bUseRTTI = true;

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
            "MediaAssets",
            "CommonUI",
             "Slate",
             "SlateCore"
        });

        AddEngineThirdPartyPrivateStaticDependencies(Target,
            "Vorbis",
            "Ogg"
        );

        PublicIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDirectory, "System"),
            Path.Combine(ModuleDirectory, "Player"),
            Path.Combine(ModuleDirectory, "Planners"),
            Path.Combine(ModuleDirectory, "Environment"),
            Path.Combine(ModuleDirectory, "Screen"),
            Path.Combine(ModuleDirectory, "Ship"),
            Path.Combine(ModuleDirectory, "Sim"),
            Path.Combine(ModuleDirectory, "AIAgent"),
            Path.Combine(ModuleDirectory, "Game"),
            Path.Combine(ModuleDirectory, "View"),
            Path.Combine(ModuleDirectory, "Display"),
            Path.Combine(ModuleDirectory, "Manager"),
            Path.Combine(ModuleDirectory, "Design"),
            Path.Combine(ModuleDirectory, "Levels"),
            Path.Combine(ModuleDirectory, "Actors"),
            Path.Combine(ModuleDirectory, "Campaign"),
            Path.Combine(ModuleDirectory, "Foundation"),
            Path.Combine(ModuleDirectory, "Combat"),
            Path.Combine(ModuleDirectory, "Mission"),
        });
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Niagara",
            "NiagaraCore",   // advanced parameter bindings
            "RenderCore",    // GPU effects, custom render logic
            "RHI"            // low-level rendering (rare)
        });
    }
}

