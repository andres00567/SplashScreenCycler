// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SplashScreenCycler : ModuleRules
{
    public SplashScreenCycler(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "DeveloperSettings",
                "EngineSettings",
                "Projects"
            }
        );
    }
}
