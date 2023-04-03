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

#include "quick_fix_command.h"

#include "app_log_wrapper.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "quick_fix_manager_client.h"
#include "status_receiver_impl.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class ApplyQuickFixMonitor : public EventFwk::CommonEventSubscriber,
                             public std::enable_shared_from_this<ApplyQuickFixMonitor> {
public:
    ApplyQuickFixMonitor(const EventFwk::CommonEventSubscribeInfo &subscribeInfo, sptr<StatusReceiverImpl> receiver)
        : EventFwk::CommonEventSubscriber(subscribeInfo), statusReceiver_(receiver)
    {}

    virtual ~ApplyQuickFixMonitor() = default;

    void OnReceiveEvent(const EventFwk::CommonEventData &eventData)
    {
        APP_LOGD("function called.");
        AAFwk::Want want = eventData.GetWant();
        int32_t applyResult = want.GetIntParam("applyResult", -1);
        std::string resultInfo = want.GetStringParam("applyResultInfo");
        std::string bundleName = want.GetStringParam("bundleName");
        APP_LOGD("bundleName: %{public}s, applyResult: %{public}d, resultInfo: %{public}s.",
            bundleName.c_str(), applyResult, resultInfo.c_str());
        resultInfo_ = resultInfo;
        statusReceiver_->OnFinished(applyResult, resultInfo);
    }

    std::string GetResultInfo()
    {
        return resultInfo_;
    }

private:
    sptr<StatusReceiverImpl> statusReceiver_ = nullptr;
    std::string resultInfo_;
};

int32_t QuickFixCommand::ApplyQuickFix(const std::vector<std::string> &quickFixFiles, std::string &resultInfo)
{
    if (quickFixFiles.empty()) {
        resultInfo.append("quick fix file is empty.\n");
        return ERR_INVALID_VALUE;
    }

    for (auto file : quickFixFiles) {
        APP_LOGI("apply hqf file %{private}s.", file.c_str());
    }

    sptr<StatusReceiverImpl> statusReceiver(new (std::nothrow) StatusReceiverImpl());
    if (statusReceiver == nullptr) {
        resultInfo.append("Create status receiver failed.\n");
        return ERR_INVALID_VALUE;
    }

    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_QUICK_FIX_APPLY_RESULT);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto applyMonitor = std::make_shared<ApplyQuickFixMonitor>(subscribeInfo, statusReceiver);
    EventFwk::CommonEventManager::SubscribeCommonEvent(applyMonitor);

    auto result = DelayedSingleton<AAFwk::QuickFixManagerClient>::GetInstance()->ApplyQuickFix(quickFixFiles);
    if (result == ERR_OK) {
        APP_LOGD("Waiting apply finished.");
        result = statusReceiver->GetResultCode();
    }

    if (result == ERR_OK) {
        resultInfo.append("apply quickfix succeed.\n");
    } else {
        if (applyMonitor->GetResultInfo().empty()) {
            resultInfo.append("apply quickfix failed with errno: " + std::to_string(result) + ".\n");
        } else {
            resultInfo.append("apply quickfix failed with error: " + applyMonitor->GetResultInfo() + ".\n");
        }
    }

    return result;
}

int32_t QuickFixCommand::GetApplyedQuickFixInfo(const std::string &bundleName, std::string &resultInfo)
{
    if (bundleName.empty()) {
        resultInfo.append("bundle name is empty.\n");
        return ERR_INVALID_VALUE;
    }

    AAFwk::ApplicationQuickFixInfo quickFixInfo;
    auto result = AAFwk::QuickFixManagerClient::GetInstance()->GetApplyedQuickFixInfo(bundleName, quickFixInfo);
    if (result == ERR_OK) {
        resultInfo.append("Information as follows:\n");
        resultInfo.append(GetQuickFixInfoString(quickFixInfo));
    } else {
        resultInfo.append("Get quick fix info failed with errno " + std::to_string(result) + ".\n");
    }

    return result;
}

std::string QuickFixCommand::GetQuickFixInfoString(const AAFwk::ApplicationQuickFixInfo &quickFixInfo)
{
    std::string info;
    info = "ApplicationQuickFixInfo:\n";
    info.append("  bundle name: " + quickFixInfo.bundleName + "\n");
    info.append("  bundle version code: " + std::to_string(quickFixInfo.bundleVersionCode) + "\n");
    info.append("  bundle version name: " + quickFixInfo.bundleVersionName + "\n");
    AppqfInfo appqfInfo = quickFixInfo.appqfInfo;
    info.append("  patch version code: " + std::to_string(appqfInfo.versionCode) + "\n");
    info.append("  patch version name: " + appqfInfo.versionName + "\n");
    info.append("  cpu abi: " + appqfInfo.cpuAbi + "\n");
    info.append("  native library path: " + appqfInfo.nativeLibraryPath + "\n");
    std::string type;
    if (appqfInfo.type == AppExecFwk::QuickFixType::PATCH) {
        type = "patch";
    } else if (appqfInfo.type == AppExecFwk::QuickFixType::HOT_RELOAD) {
        type = "hotreload";
    }
    info.append("  type: " + type + "\n");
    for (auto hqfInfo : appqfInfo.hqfInfos) {
        info.append("  ModuelQuickFixInfo:\n");
        info.append("    module name: " + hqfInfo.moduleName + "\n");
        info.append("    module sha256: " + hqfInfo.hapSha256 + "\n");
        info.append("    file path: " + hqfInfo.hqfFilePath + "\n");
    }

    return info;
}
} // namespace AppExecFwk
} // namespace OHOS