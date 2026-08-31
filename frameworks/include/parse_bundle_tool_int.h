/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_PARSE_BUNDLE_TOOL_INT_H
#define FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_PARSE_BUNDLE_TOOL_INT_H

#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

namespace OHOS {
namespace AppExecFwk {
/*
 * Parse a whole-token decimal integer from bundle_test_tool CLI text.
 * Reject empty, overflow, underflow, leftover partial, junk, '+', hex, and floats.
 * Valid in-range values keep the same numeric result as std::stoi / std::stoull
 * on fully-consumed digit-only input.
 */
inline bool ParseBundleToolInt32(std::string_view text, int32_t &out)
{
    if (text.empty()) {
        return false;
    }
    int32_t value = 0;
    auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc() || result.ptr != text.data() + text.size()) {
        return false;
    }
    out = value;
    return true;
}

inline bool ParseBundleToolInt32(const std::string &text, int32_t &out)
{
    return ParseBundleToolInt32(std::string_view(text), out);
}

inline bool ParseBundleToolUint64(std::string_view text, uint64_t &out)
{
    if (text.empty()) {
        return false;
    }
    uint64_t value = 0;
    auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc() || result.ptr != text.data() + text.size()) {
        return false;
    }
    out = value;
    return true;
}

inline bool ParseBundleToolUint64(const std::string &text, uint64_t &out)
{
    return ParseBundleToolUint64(std::string_view(text), out);
}
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_PARSE_BUNDLE_TOOL_INT_H
