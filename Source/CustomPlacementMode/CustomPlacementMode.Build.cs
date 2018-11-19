// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class CustomPlacementMode : ModuleRules
	{
        public CustomPlacementMode(ReadOnlyTargetRules Target)
            : base(Target)
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateIncludePaths.Add("CustomPlacementMode/Private");

			PublicDependencyModuleNames.AddRange(
				new string[] {
                    "Core",
                    "CoreUObject",
                    "InputCore",
                    "Engine",
                    "Settings",
                    "ClassViewer"
                }
			);

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"APPFRAMEWORK",
                    "Slate",
                    "SlateCore",
                    "PropertyEditor",
                    "UnrealEd",
                    "EditorStyle",
                    "GraphEditor",
                    "ContentBrowser",
                    "RenderCore"
                }
			);

			// Uncomment if you are using Slate UI
			PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

            PrivateIncludePaths.AddRange(
            new string[] {
            }
            );


            PrivateIncludePathModuleNames.AddRange(
                new string[] {
                }
                );
    }
	}
}
