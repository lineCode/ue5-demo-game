// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "XboxGDKBlackBoxCrashHandler.h"
#include "BlackBoxCommon.h"
#include "PooledMalloc.h"

#if BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX
#    include "XboxCommonPlatformCrashContext.h"
#endif

#if BLACKBOX_UE_XBOXONEGDK
#    if ENGINE_MAJOR_VERSION == 4
#        include "XboxOneGDKPlatformProcess.h"
#    else
#        include "XboxCommonPlatformProcess.h"
#    endif
#elif BLACKBOX_UE_XSX
#    include "XSXPlatformProcess.h"
#endif

#include "accelbyte/cpp/blackbox.h"

FXboxGDKBlackBoxCrashHandler::FXboxGDKBlackBoxCrashHandler()
{
#if defined(BLACKBOX_ENABLE_XBOX_GDK_CRASH_REPORT)
    FPooledMalloc::Get(50 * 1024 * 1024);
    FPlatformCrashContext::SetCrashVideoFn(&blackbox::save_crash_video);
    auto BlackboxCustomCrashJob = [](const TCHAR* MiniDumpPath,
                                     const TCHAR* CrashLogPath,
                                     void* RuntimeXmlData,
                                     size_t RuntimeXmlSize,
                                     char* Stacktrace,
                                     size_t StacktraceSize) {
        blackbox::handle_crash(MiniDumpPath, CrashLogPath, RuntimeXmlData, RuntimeXmlSize, Stacktrace, StacktraceSize);
        auto IsConfigValid = blackbox::validate_config();
        if (IsConfigValid) {
            auto OriginalCrashMalloc = GMalloc;
            GMalloc = &FPooledMalloc::Get();
            blackbox::send_crash(TCHAR_TO_ANSI(TEXT("D:\\")), nullptr, true);
            GMalloc = OriginalCrashMalloc;
        };
    };
    FPlatformCrashContext::SetCustomCrashJob(BlackboxCustomCrashJob);
#endif
}

FXboxGDKBlackBoxCrashHandler::~FXboxGDKBlackBoxCrashHandler()
{
}
