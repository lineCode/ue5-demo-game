// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#include <cstdint>
#include <string>

namespace blackbox {
struct callback_http_response {
    uint32_t http_status;
    bool is_success;
    const char* trace_id;
};
} // namespace blackbox