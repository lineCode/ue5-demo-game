// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#ifndef BLACKBOX_UE_XBOXXDK_BLACKBOX_CRASH_HANDLER_H_
#define BLACKBOX_UE_XBOXXDK_BLACKBOX_CRASH_HANDLER_H_

class FXboxXDKBlackBoxCrashHandler {
public:
    FXboxXDKBlackBoxCrashHandler();
    ~FXboxXDKBlackBoxCrashHandler();
    FXboxXDKBlackBoxCrashHandler(FXboxXDKBlackBoxCrashHandler&& other) = delete;
    FXboxXDKBlackBoxCrashHandler& operator=(FXboxXDKBlackBoxCrashHandler&& other) = delete;
    FXboxXDKBlackBoxCrashHandler(const FXboxXDKBlackBoxCrashHandler& other) = delete;
    FXboxXDKBlackBoxCrashHandler& operator=(const FXboxXDKBlackBoxCrashHandler& other) = delete;
};
#endif