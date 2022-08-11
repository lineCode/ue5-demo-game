// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#ifndef BLACKBOX_UE_MAC_BLACKBOX_CRASH_HANDLER_H_
#define BLACKBOX_UE_MAC_BLACKBOX_CRASH_HANDLER_H_

#include "Mac/MacPlatformCrashContext.h"

class FMacBlackBoxCrashHandler {
public:
    FMacBlackBoxCrashHandler();
    ~FMacBlackBoxCrashHandler() = default;
    FMacBlackBoxCrashHandler(FMacBlackBoxCrashHandler&& other) = delete;
    FMacBlackBoxCrashHandler& operator=(FMacBlackBoxCrashHandler&& other) = delete;
    FMacBlackBoxCrashHandler(const FMacBlackBoxCrashHandler& other) = delete;
    FMacBlackBoxCrashHandler& operator=(const FMacBlackBoxCrashHandler& other) = delete;

private:
    static void OnGameCrash(const FGenericCrashContext& Context);
};
#endif