// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ARENA_LASTGATE : ModuleRules
{
	public ARENA_LASTGATE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] {
			ModuleDirectory
		});

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore",
			"EnhancedInput", "GameplayAbilities", "GameplayTags", "GameplayTasks",

			//Session
			"OnlineSubsystem",
			"OnlineSubsystemUtils",

			//UI 관련
			"UMG",
            "Slate",
            "SlateCore",
            "NavigationSystem",

			// Niagara
			"Niagara",
			"NiagaraCore",

            "AnimGraphRuntime",

            "LevelSequence",
            "MovieScene"
        });

		// Editor 전용 모듈 (에디터에서만 사용)
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				"UnrealEd",        // 에디터 기본 기능
				"Blutility",       // Editor Utility Widget
				"AssetTools",      // EditorAssetLibrary
				"ContentBrowser"   // 콘텐츠 브라우저 접근
			});
		}

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
