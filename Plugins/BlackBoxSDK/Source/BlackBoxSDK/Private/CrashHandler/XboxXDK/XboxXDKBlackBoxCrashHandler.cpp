// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "XboxXDKBlackBoxCrashHandler.h"
#include "BlackBoxCommon.h"

#if BLACKBOX_UE_XBOXONE
#    if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION < 26)
#        include "XboxOne/XboxOnePlatformCrashContext.h"
#        include "XboxOne/XboxOnePlatformProcess.h"
#    else
#        include "XboxOnePlatformCrashContext.h"
#        include "XboxOnePlatformProcess.h"
#    endif
#endif

#include "accelbyte/cpp/blackbox.h"

FXboxXDKBlackBoxCrashHandler::FXboxXDKBlackBoxCrashHandler()
{
}

FXboxXDKBlackBoxCrashHandler::~FXboxXDKBlackBoxCrashHandler()
{
}