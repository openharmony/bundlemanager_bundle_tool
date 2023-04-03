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

#include "bundle_tool_callback_stub.h"

#include "app_log_wrapper.h"
#include "message_parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
int32_t BundleToolCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    APP_LOGI("bundle mgr callback onReceived message, the message code is %{public}u", code);
    std::unique_lock<std::mutex> lock(mutex_);
    std::u16string descriptor = BundleToolCallbackStub::GetDescriptor();
    std::u16string token = data.ReadInterfaceToken();
    if (descriptor != token) {
        APP_LOGE("failed to verify parameters");
        return OBJECT_NULL;
    }
    int32_t resultCode = data.ReadInt32();
    uint32_t versionCode = data.ReadUint32();
    int32_t missionId = data.ReadInt32();
    APP_LOGI("BundleToolCallbackStub: resultCode:%{public}d, versionCode:%{public}d, missionId:%{public}d",
        resultCode, versionCode, missionId);
    dataReady_ = true;
    cv_.notify_all();
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS