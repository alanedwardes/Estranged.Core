using System;
using System.IO;

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

            if (Target.Platform == UnrealTargetPlatform.Win64 ||
                Target.Platform == UnrealTargetPlatform.Win32)
            {
                PublicIncludePaths.Add(IncludePathSDL2);
                PublicAdditionalLibraries.Add(Path.Combine(LibraryPathSDL2, "SDL2.lib"));
                RuntimeDependencies.Add(Path.Combine(BinaryPathSDL2, "SDL2.dll"));
                PublicDelayLoadDLLs.Add("SDL2.dll");
            }
        }

        private string ThirdPartyPath
        {
            get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
        }

        private string BinaryPathSDL2
        {
            get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "SDL2", "bin")); }
        }

        private string LibraryPathSDL2
        {
            get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "SDL2", "lib")); }
        }

        private string IncludePathSDL2
        {
            get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "SDL2", "include")); }
        }
    }
}