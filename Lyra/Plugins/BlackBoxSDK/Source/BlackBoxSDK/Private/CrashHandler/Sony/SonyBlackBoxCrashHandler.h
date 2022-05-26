// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#ifndef BLACKBOX_UE_SONY_BLACKBOX_CRASH_HANDLER_H_
#define BLACKBOX_UE_SONY_BLACKBOX_CRASH_HANDLER_H_

class FSonyBlackBoxCrashHandler {
public:
    FSonyBlackBoxCrashHandler();
    ~FSonyBlackBoxCrashHandler() = default;
    FSonyBlackBoxCrashHandler(FSonyBlackBoxCrashHandler&& other) = delete;
    FSonyBlackBoxCrashHandler& operator=(FSonyBlackBoxCrashHandler&& other) = delete;
    FSonyBlackBoxCrashHandler(const FSonyBlackBoxCrashHandler& other) = delete;
    FSonyBlackBoxCrashHandler& operator=(const FSonyBlackBoxCrashHandler& other) = delete;
};
#endif