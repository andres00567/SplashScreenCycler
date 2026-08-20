// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SplashScreenCyclerSettings.generated.h"

UCLASS(config=SplashScreenCycler, defaultconfig, meta=(DisplayName="Splash Screen Cycler"))
class SPLASHSCREENCYCLER_API USplashScreenCyclerSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    USplashScreenCyclerSettings();

    virtual FName GetContainerName() const override;
    virtual FName GetCategoryName() const override;
    virtual FName GetSectionName() const override;

    UPROPERTY(config, EditAnywhere, Category="Cycling")
    bool bCycleOnEditorStartup;

    UPROPERTY(config, EditAnywhere, Category="Cycling")
    bool bCycleOnRuntimeStartup;

    UPROPERTY(config, EditAnywhere, Category="Cycling", meta=(FilePathFilter="png"))
    TArray<FFilePath> SplashScreens;

    UPROPERTY(config, EditAnywhere, Category="Cycling")
    FDirectoryPath FallbackSplashImagesDirectory;

    UPROPERTY(config, EditAnywhere, Category="Output", meta=(RelativePath))
    FFilePath TargetEditorSplashPath;

    UPROPERTY(config, EditAnywhere, Category="Output", meta=(RelativePath))
    FFilePath TargetRuntimeSplashPath;

    UPROPERTY(config, EditAnywhere, Category="Launch Metadata")
    bool bSyncLaunchMetadataToProjectSettings;

    UPROPERTY(config, EditAnywhere, Category="Launch Metadata")
    FString ProjectDisplayName;

    UPROPERTY(config, EditAnywhere, Category="Launch Metadata")
    FString ProjectVersion;

    UPROPERTY(config, EditAnywhere, Category="Launch Metadata")
    FString StudioName;

    UPROPERTY(config, EditAnywhere, Category="Launch Metadata")
    FString StudioHomepage;

    UPROPERTY(config, EditAnywhere, Category="Launch Metadata", meta=(MultiLine=true))
    FString CopyrightNotice;
};
