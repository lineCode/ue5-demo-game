// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#ifndef BLACKBOX_UE_XBOXGDK_BLACKBOX_CRASH_HANDLER_H_
#define BLACKBOX_UE_XBOXGDK_BLACKBOX_CRASH_HANDLER_H_

class FXboxGDKBlackBoxCrashHandler {
public:
    FXboxGDKBlackBoxCrashHandler();
    ~FXboxGDKBlackBoxCrashHandler();
    FXboxGDKBlackBoxCrashHandler(FXboxGDKBlackBoxCrashHandler&& other) = delete;
    FXboxGDKBlackBoxCrashHandler& operator=(FXboxGDKBlackBoxCrashHandler&& other) = delete;
    FXboxGDKBlackBoxCrashHandler(const FXboxGDKBlackBoxCrashHandler& other) = delete;
    FXboxGDKBlackBoxCrashHandler& operator=(const FXboxGDKBlackBoxCrashHandler& other) = delete;
};
#endif