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

#ifndef FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_BUNDLE_COMMAND_COMMON_H
#define FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_BUNDLE_COMMAND_COMMON_H

#include "bundle_mgr_interface.h"
#include "bundle_installer_interface.h"

#include <map>
#include <string>

namespace OHOS {
namespace AppExecFwk {
class BundleCommandCommon {
public:
    static sptr<IBundleMgr> GetBundleMgrProxy();

    static int32_t GetCurrentUserId(int32_t userId);
    static bool IsUserForeground(int32_t userId);
    static int32_t GetOsAccountLocalIdFromUid(const int32_t callingUid);

    static std::map<int32_t, std::string> bundleMessageMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_BUNDLE_COMMAND_COMMON_H
