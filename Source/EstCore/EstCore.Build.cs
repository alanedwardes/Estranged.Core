namespace UnrealBuildTool.Rules
{
    public class EstCore : ModuleRules
    {
        public EstCore(ReadOnlyTargetRules Target) : base(Target)
        {
            PrivatePCHHeaderFile = "Public/EstCore.h";

            PublicDependencyModuleNames.AddRange(
                new string[] {
                    "Core",
                    "Landscape",
                    "JsonUtilities",
                    "EngineSettings",
                    "InputCore",
                    "CoreUObject",
                    "PhysicsCore",
                    "AnimGraphRuntime",
                    "MovieScene",
                    "LevelSequence",
                    "Engine",
                    "UMG",
                    "Slate",
                    "SlateCore",
                    "AIModule",
                    "HTTP",
                    "RenderCore"
            });
        }
    }
}