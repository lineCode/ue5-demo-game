// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_BLACKBOX_ERROR_CODES_H
#define ACCELBYTE_BLACKBOX_ERROR_CODES_H

enum blackbox_return {
    NO_ERROR_ = 0,
    UNABLE_TO_GET_STAGING_TEXTURE,
    UNABLE_TO_INITIATE_SHARED_MEMORY,
    MISSING_SHARED_MEMORY_MANAGER,
    MISSING_CRASH_VIDEO_RECORDER,
    INSUFFICIENT_BUFFER,
    HTTP_WORKER_THREAD_IS_MISSING,
    CALLBACK_QUEUE_THREAD_IS_MISSING,
    NO_CALLBACK_TO_CALL,
    INVALID_URL_PATH_PARAMETER_GIVEN
};

#endif // !ACCELBYTE_BLACKBOX_ERROR_CODES_H
