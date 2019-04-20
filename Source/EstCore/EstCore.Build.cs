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
                    "InputCore",
                    "CoreUObject",
                    "MovieScene",
                    "LevelSequence",
                    "Engine",
                    "UMG",
                    "Slate",
                    "SlateCore",
                    "AIModule",
                    "PhysX",
                    "APEX",
                    "RenderCore"
            });
        }
    }
}