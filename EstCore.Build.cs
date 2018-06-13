using UnrealBuildTool;

public class EstCore : ModuleRules
{
    public EstCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "InputCore",
                "CoreUObject",
                "MovieScene",
                "LevelSequence",
                "Engine",
                "UMG",
                "Slate",
                "AIModule"
        });
    }
}