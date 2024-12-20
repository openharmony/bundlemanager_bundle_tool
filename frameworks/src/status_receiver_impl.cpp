/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "status_receiver_impl.h"

#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t MINIMUM_WAITTING_TIME = 180; // 3 mins
} // namespace

StatusReceiverImpl::StatusReceiverImpl(int32_t waittingTime) : waittingTime_(waittingTime)
{
    APP_LOGI("create status receiver instance");
}

StatusReceiverImpl::StatusReceiverImpl() : waittingTime_(MINIMUM_WAITTING_TIME)
{
    APP_LOGI("create status receiver instance");
}

StatusReceiverImpl::~StatusReceiverImpl()
{
    APP_LOGI("destroy status receiver instance");
}

void StatusReceiverImpl::OnFinished(const int32_t resultCode, const std::string &resultMsg)
{
    APP_LOGI("on finished result is %{public}d, %{public}s", resultCode, resultMsg.c_str());
    std::lock_guard<std::mutex> lock(setValueMutex_);
    if (!isSetValue) {
        isSetValue = true;
        resultCodeSignal_.set_value(resultCode);
        resultMsgSignal_.set_value(resultMsg);
    } else {
        APP_LOGW("resultCodeSignal_ is set");
    }
}

void StatusReceiverImpl::OnStatusNotify(const int progress)
{
    APP_LOGI("on OnStatusNotify is %{public}d", progress);
}

int32_t StatusReceiverImpl::GetResultCode() const
{
    auto future = resultCodeSignal_.get_future();
    if (future.wait_for(std::chrono::seconds(waittingTime_)) == std::future_status::ready) {
        int32_t resultCode = future.get();
        return resultCode;
    }
    return ERR_OPERATION_TIME_OUT;
}

std::string StatusReceiverImpl::GetResultMsg() const
{
    auto future = resultMsgSignal_.get_future();
    if (future.wait_for(std::chrono::seconds(waittingTime_)) == std::future_status::ready) {
        std::string resultMsg = future.get();
        return resultMsg;
    }
    return "";
}
}  // namespace AppExecFwk
}  // namespace OHOS