// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NauseaDungeon : ModuleRules
{
	public NauseaDungeon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] { "NauseaDungeon", "NauseaDungeon/Public" });
        PrivateIncludePaths.AddRange(new string[] { "NauseaDungeon/Private" });

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "NetCore", "PhysicsCore", "InputCore" });
        PublicDependencyModuleNames.AddRange(new string[] { "AIModule", "NavigationSystem" });
        PublicDependencyModuleNames.AddRange(new string[] { "SlateCore", "Slate", "UMG" });
        PublicDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "GameplayTasks", "EngineSettings", "AnimationBudgetAllocator" });

        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(new string[] { "RenderCore", "RHI", "UnrealEd" });
            PublicDependencyModuleNames.AddRange(new string[] { "GameplayDebugger" });
        }
    }
}
