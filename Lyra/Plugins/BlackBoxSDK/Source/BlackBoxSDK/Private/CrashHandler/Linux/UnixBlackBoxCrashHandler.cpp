// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "UnixBlackBoxCrashHandler.h"

#include "accelbyte/cpp/blackbox.h"
#include "HAL/ThreadHeartBeat.h"
#include "Misc/FileHelper.h"
#include "Unix/UnixPlatformProcess.h"
#include "Unix/UnixPlatformMisc.h"
#include "Misc/FeedbackContext.h"

#include "BlackBoxLog.h"

FUnixBlackBoxCrashHandler::FUnixBlackBoxCrashHandler()
{
    blackbox::init_crash_handler(blackbox::config_get_crash_folder());
    FPlatformMisc::SetCrashHandler(&FUnixBlackBoxCrashHandler::OnGameCrash);
}

FUnixBlackBoxCrashHandler::~FUnixBlackBoxCrashHandler()
{
    // Nothing to do
}

void FUnixBlackBoxCrashHandler::OnGameCrash(const FGenericCrashContext& Context)
{
    const FUnixCrashContext& UnixContext = static_cast<const FUnixCrashContext&>(Context);

    FString CrashFolder = UTF8_TO_TCHAR(blackbox::config_get_crash_folder());

    if (IFileManager::Get().MakeDirectory(*CrashFolder, true)) {
        blackbox::ucontext_buffer* buff = reinterpret_cast<blackbox::ucontext_buffer*>(UnixContext.Context);
        blackbox::handle_crash(UnixContext.Signal, UnixContext.Info, buff);
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Failed to create crash folder"));
    }

    FThreadHeartBeat::Get().Stop();
#if (ENGINE_MAJOR_VERSION <= 4)
    const_cast<FUnixCrashContext&>(UnixContext).CaptureStackTrace();
#else
    const_cast<FUnixCrashContext&>(UnixContext).CaptureStackTrace(UnixContext.ErrorFrame);
#endif
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

    return UnixContext.GenerateCrashInfoAndLaunchReporter();
}
