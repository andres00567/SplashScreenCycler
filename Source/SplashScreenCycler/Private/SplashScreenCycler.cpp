// Copyright Epic Games, Inc. All Rights Reserved.

#include "SplashScreenCycler.h"

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "SplashScreenCyclerSettings.h"

#if WITH_EDITOR
#include "GeneralProjectSettings.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogSplashScreenCycler, Log, All);

namespace SplashScreenCycler
{
    const TCHAR* PluginName = TEXT("SplashScreenCycler");
    const TCHAR* StateRelativePath = TEXT("SplashScreenCycler/LastSplash.txt");

    struct FSplashCopyTarget
    {
        FString Name;
        FString DefaultPath;
        FFilePath ConfiguredPath;
    };

    bool IsSupportedSplashImage(const FString& FilePath)
    {
        return FPaths::GetExtension(FilePath).Equals(TEXT("png"), ESearchCase::IgnoreCase);
    }

    FString ResolveProjectRelativePath(const FString& Path)
    {
        if (FPaths::IsRelative(Path))
        {
            return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), Path));
        }

        return FPaths::ConvertRelativePathToFull(Path);
    }

    void AddConfiguredSplashImage(TArray<FString>& SplashImages, const FFilePath& SplashScreen)
    {
        if (SplashScreen.FilePath.IsEmpty())
        {
            return;
        }

        const FString ResolvedPath = ResolveProjectRelativePath(SplashScreen.FilePath);
        if (IsSupportedSplashImage(ResolvedPath) && FPaths::FileExists(ResolvedPath))
        {
            SplashImages.AddUnique(ResolvedPath);
        }
        else
        {
            UE_LOG(LogSplashScreenCycler, Warning, TEXT("Configured splash image is missing or not a PNG: %s."), *SplashScreen.FilePath);
        }
    }

    bool CopySplashImageToTarget(const FString& SourceSplashImage, const FSplashCopyTarget& Target)
    {
        FString TargetSplashPath = Target.ConfiguredPath.FilePath;
        if (TargetSplashPath.IsEmpty())
        {
            TargetSplashPath = Target.DefaultPath;
        }

        TargetSplashPath = ResolveProjectRelativePath(TargetSplashPath);
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(TargetSplashPath), true);

        const uint32 CopyResult = IFileManager::Get().Copy(
            *TargetSplashPath,
            *SourceSplashImage,
            true,
            true
        );

        if (CopyResult != COPY_OK)
        {
            UE_LOG(LogSplashScreenCycler, Warning, TEXT("Failed to copy %s splash image from %s to %s."), *Target.Name, *SourceSplashImage, *TargetSplashPath);
            return false;
        }

        UE_LOG(LogSplashScreenCycler, Log, TEXT("Prepared %s splash image for next startup: %s."), *Target.Name, *SourceSplashImage);
        return true;
    }

}

void FSplashScreenCyclerModule::StartupModule()
{
#if WITH_EDITOR
    SyncLaunchMetadata();
#endif
    CycleSplashImages();
}

void FSplashScreenCyclerModule::ShutdownModule()
{
}

void FSplashScreenCyclerModule::CycleSplashImages() const
{
    const USplashScreenCyclerSettings* Settings = GetDefault<USplashScreenCyclerSettings>();
    if (!Settings || (!Settings->bCycleOnEditorStartup && !Settings->bCycleOnRuntimeStartup))
    {
        return;
    }

    TArray<FString> SplashImages;
    for (const FFilePath& SplashScreen : Settings->SplashScreens)
    {
        SplashScreenCycler::AddConfiguredSplashImage(SplashImages, SplashScreen);
    }

    if (SplashImages.Num() == 0 && !Settings->FallbackSplashImagesDirectory.Path.IsEmpty())
    {
        const FString SourceDirectory = SplashScreenCycler::ResolveProjectRelativePath(Settings->FallbackSplashImagesDirectory.Path);
        IFileManager::Get().FindFilesRecursive(
            SplashImages,
            *SourceDirectory,
            TEXT("*.png"),
            true,
            false
        );
    }

    SplashImages.Sort([](const FString& Left, const FString& Right)
    {
        return Left < Right;
    });

    if (SplashImages.Num() == 0)
    {
        UE_LOG(LogSplashScreenCycler, Warning, TEXT("No configured or fallback PNG splash images found."));
        return;
    }

    const FString StatePath = FPaths::Combine(FPaths::ProjectSavedDir(), SplashScreenCycler::StateRelativePath);
    FString LastSplashImage;
    FFileHelper::LoadFileToString(LastSplashImage, *StatePath);
    LastSplashImage.TrimStartAndEndInline();

    int32 LastSplashIndex = SplashImages.IndexOfByPredicate([&LastSplashImage](const FString& SplashImage)
    {
        return SplashImage.Equals(LastSplashImage, ESearchCase::IgnoreCase);
    });

    const int32 NextSplashIndex = (LastSplashIndex + 1) % SplashImages.Num();
    const FString& NextSplashImage = SplashImages[NextSplashIndex];

    if (!SplashScreenCycler::IsSupportedSplashImage(NextSplashImage))
    {
        UE_LOG(LogSplashScreenCycler, Warning, TEXT("Unsupported splash image skipped: %s."), *NextSplashImage);
        return;
    }

    bool bCopiedAnyTarget = false;

    if (Settings->bCycleOnEditorStartup)
    {
        bCopiedAnyTarget |= SplashScreenCycler::CopySplashImageToTarget(
            NextSplashImage,
            { TEXT("editor"), TEXT("Content/Splash/EdSplash.png"), Settings->TargetEditorSplashPath }
        );
    }

    if (Settings->bCycleOnRuntimeStartup)
    {
        bCopiedAnyTarget |= SplashScreenCycler::CopySplashImageToTarget(
            NextSplashImage,
            { TEXT("runtime"), TEXT("Content/Splash/Splash.png"), Settings->TargetRuntimeSplashPath }
        );
    }

    if (!bCopiedAnyTarget)
    {
        UE_LOG(LogSplashScreenCycler, Warning, TEXT("Failed to copy splash image to any enabled target."));
        return;
    }

    IFileManager::Get().MakeDirectory(*FPaths::GetPath(StatePath), true);
    if (!FFileHelper::SaveStringToFile(NextSplashImage, *StatePath))
    {
        UE_LOG(LogSplashScreenCycler, Warning, TEXT("Failed to save splash cycler state to %s."), *StatePath);
    }
}

void FSplashScreenCyclerModule::SyncLaunchMetadata() const
{
#if WITH_EDITOR
    const USplashScreenCyclerSettings* Settings = GetDefault<USplashScreenCyclerSettings>();
    if (!Settings || !Settings->bSyncLaunchMetadataToProjectSettings)
    {
        return;
    }

    UGeneralProjectSettings* ProjectSettings = GetMutableDefault<UGeneralProjectSettings>();
    if (!ProjectSettings)
    {
        return;
    }

    if (!Settings->ProjectDisplayName.IsEmpty())
    {
        ProjectSettings->ProjectName = Settings->ProjectDisplayName;
        ProjectSettings->ProjectDisplayedTitle = FText::FromString(Settings->ProjectDisplayName);
    }

    if (!Settings->ProjectVersion.IsEmpty())
    {
        ProjectSettings->ProjectVersion = Settings->ProjectVersion;
    }

    if (!Settings->StudioName.IsEmpty())
    {
        ProjectSettings->CompanyName = Settings->StudioName;
    }

    if (!Settings->StudioHomepage.IsEmpty())
    {
        ProjectSettings->Homepage = Settings->StudioHomepage;
    }

    if (!Settings->CopyrightNotice.IsEmpty())
    {
        ProjectSettings->CopyrightNotice = Settings->CopyrightNotice;
    }

    ProjectSettings->SaveConfig();
#endif
}

IMPLEMENT_MODULE(FSplashScreenCyclerModule, SplashScreenCycler)
