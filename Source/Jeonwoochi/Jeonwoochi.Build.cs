// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Jeonwoochi : ModuleRules
{
    public Jeonwoochi(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 런타임(Shipping)에서 써도 되는 모듈만 남김
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject",
            "Engine", "InputCore",
            "EnhancedInput", "AIModule",
            "GameplayTags", "MotionWarping",
            "GameplayTasks",
            "Niagara", "AnimGraphRuntime",
            "LevelSequence", "MovieScene",
            "CinematicCamera",
            "GeometryCollectionEngine", "MediaAssets","UMG"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "GASP",
            "GASPCamera",
            "Slate", "SlateCore",         // 런타임에서 사용 가능
            "Chaos", "ChaosSolverEngine",
            "FieldSystemEngine",
            "AsyncLoadingScreen",

            "HairStrandsCore"
        });

        // 에디터 전용 모듈은 여기로
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "UnrealEd",
                "InterchangeCore",
                "InterchangeEngine",
                "InterchangeImport"
            });
        }
    }
}
