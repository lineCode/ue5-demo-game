// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SonyBlackBoxCrashHandler.h"
#include "accelbyte/cpp/blackbox.h"
#include "BlackBoxCommon.h"

FSonyBlackBoxCrashHandler::FSonyBlackBoxCrashHandler()
{
#if BLACKBOX_UE_SONY
    blackbox::init_crash_handler();
#endif
}
