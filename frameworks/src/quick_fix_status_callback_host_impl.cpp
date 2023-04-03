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

#include "quick_fix_status_callback_host_impl.h"

#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t WAITTING_TIME = 5;
} // namespace

QuickFixStatusCallbackHostlmpl::QuickFixStatusCallbackHostlmpl()
{
    APP_LOGI("create status callback instance");
}

QuickFixStatusCallbackHostlmpl::~QuickFixStatusCallbackHostlmpl()
{
    APP_LOGI("destroy status callback instance");
}

void QuickFixStatusCallbackHostlmpl::OnPatchDeployed(const std::shared_ptr<QuickFixResult> &result)
{
    APP_LOGD("on Patch deployed result is %{public}s", result->ToString().c_str());
    quickFixPromise_.set_value(result);
}

void QuickFixStatusCallbackHostlmpl::OnPatchSwitched(const std::shared_ptr<QuickFixResult> &result)
{
    APP_LOGD("on Patch switched result is %{public}s", result->ToString().c_str());
    quickFixPromise_.set_value(result);
}

void QuickFixStatusCallbackHostlmpl::OnPatchDeleted(const std::shared_ptr<QuickFixResult> &result)
{
    APP_LOGD("on Patch deleted result is %{public}s", result->ToString().c_str());
    quickFixPromise_.set_value(result);
}

ErrCode QuickFixStatusCallbackHostlmpl::GetResultCode(std::shared_ptr<QuickFixResult> &quickFixRes) const
{
    auto future = quickFixPromise_.get_future();
    if (future.wait_for(std::chrono::seconds(WAITTING_TIME)) == std::future_status::ready) {
        quickFixRes = future.get();
        return quickFixRes->GetResCode();
    }
    return ERR_APPEXECFWK_OPERATION_TIME_OUT;
}
}  // namespace AppExecFwk
}  // namespace OHOS