// Copyright Epic Games, Inc. All Rights Reserved.

#include "SplashScreenCyclerSettings.h"

USplashScreenCyclerSettings::USplashScreenCyclerSettings()
    : bCycleOnEditorStartup(true)
    , bCycleOnRuntimeStartup(true)
    , bSyncLaunchMetadataToProjectSettings(false)
    , ProjectDisplayName(TEXT("Hell Run"))
{
    FallbackSplashImagesDirectory.Path = TEXT("Plugins/SplashScreenCycler/Content/SplashImages");
    TargetEditorSplashPath.FilePath = TEXT("Content/Splash/EdSplash.png");
    TargetRuntimeSplashPath.FilePath = TEXT("Content/Splash/Splash.png");
}

FName USplashScreenCyclerSettings::GetContainerName() const
{
    return TEXT("Project");
}

FName USplashScreenCyclerSettings::GetCategoryName() const
{
    return TEXT("Plugins");
}

FName USplashScreenCyclerSettings::GetSectionName() const
{
    return TEXT("SplashScreenCycler");
}
