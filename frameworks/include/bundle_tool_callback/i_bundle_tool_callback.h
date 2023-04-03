/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_BUNDLEMANAGER_BUNDLE_FRAMEWORK_BUNDLE_TOOL_INCLUDE_I_BUNDLE_TOOL_CALLBACK_H
#define FOUNDATION_BUNDLEMANAGER_BUNDLE_FRAMEWORK_BUNDLE_TOOL_INCLUDE_I_BUNDLE_TOOL_CALLBACK_H

#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class IBundleToolCallback
 * IBundleToolCallback is used to notify caller ability that query complete.
 */
class IBundleToolCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedSchedule.IDmsBundleManagerCallback");
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_BUNDLEMANAGER_BUNDLE_FRAMEWORK_BUNDLE_TOOL_INCLUDE_I_BUNDLE_TOOL_CALLBACK_H
