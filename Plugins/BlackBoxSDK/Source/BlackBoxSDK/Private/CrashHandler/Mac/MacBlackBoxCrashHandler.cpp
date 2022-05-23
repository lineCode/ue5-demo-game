// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "MacBlackBoxCrashHandler.h"

#include "Misc/FileHelper.h"
#include "accelbyte/cpp/blackbox.h"

FMacBlackBoxCrashHandler::FMacBlackBoxCrashHandler()
{
    blackbox::init_crash_handler(blackbox::config_get_crash_folder());
    FPlatformMisc::SetCrashHandler(&FMacBlackBoxCrashHandler::OnGameCrash);
}

void FMacBlackBoxCrashHandler::OnGameCrash(const FGenericCrashContext& Context)
{
    const FMacCrashContext& MacContext = static_cast<const FMacCrashContext&>(Context);

    FString CrashFolder = UTF8_TO_TCHAR(blackbox::config_get_crash_folder());

    if (IFileManager::Get().MakeDirectory(*CrashFolder, true)) {
        blackbox::handle_crash();
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Failed to create crash folder"));
    }

    FThreadHeartBeat::Get().Stop();

    if (GLog) {
        GLog->SetCurrentThreadAsMasterThread();
        GLog->Flush();
    }
    if (GWarn) {
        GWarn->Flush();
    }
    if (GError) {
        GError->Flush();
        GError->HandleError();
    }

    return MacContext.GenerateCrashInfoAndLaunchReporter();
}
