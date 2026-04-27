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

#ifndef FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_ERROR_CODE_UTILS_H
#define FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_ERROR_CODE_UTILS_H

#include <string>

namespace OHOS {
namespace AppExecFwk {

class ErrorCodeUtils {
public:
    /**
     * Convert error code to error code string (enum literal name).
     * @param code The error code (int32_t).
     * @return The error code string, e.g. "ERR_INSTALL_INTERNAL_ERROR".
     */
    static std::string GetErrorCodeString(int32_t code);

private:
    ErrorCodeUtils() = delete;
    ~ErrorCodeUtils() = delete;
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_ERROR_CODE_UTILS_H