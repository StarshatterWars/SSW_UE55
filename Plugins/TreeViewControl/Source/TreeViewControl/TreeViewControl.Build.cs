// Copyright 2018, Sameek Kundu. All Rights Reserved. 

using UnrealBuildTool;

public class TreeViewControl : ModuleRules
{
	public TreeViewControl(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore" , "Projects"});

        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.Add("EditorStyle");

        }
        
        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
