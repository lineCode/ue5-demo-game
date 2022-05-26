// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#include <cstdint>

namespace blackbox {
namespace log {
enum severity : uint8_t { ERROR_ = 0x01, WARNING = 0x02, INFO = 0x04, VERBOSE = 0x08 };
} // namespace log
} // namespace blackbox
