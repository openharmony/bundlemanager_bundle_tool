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

#ifndef FOUNDATION_BUNDLEMANAGER_BUNDLE_FRAMEWORK_BUNDLE_TOOL_INCLUDE_BUNDLE_TEST_TOOL_H
#define FOUNDATION_BUNDLEMANAGER_BUNDLE_FRAMEWORK_BUNDLE_TOOL_INCLUDE_BUNDLE_TEST_TOOL_H

#include "shell_command.h"
#include "bundle_event_callback_host.h"
#include "bundle_mgr_interface.h"
#include "bundle_installer_interface.h"
#ifdef DISTRIBUTED_BUNDLE_FRAMEWORK
#include "distributed_bms_interface.h"
#endif

namespace OHOS {
namespace AppExecFwk {
class BundleEventCallbackImpl : public BundleEventCallbackHost {
public:
    BundleEventCallbackImpl();
    virtual ~BundleEventCallbackImpl() override;
    virtual void OnReceiveEvent(const EventFwk::CommonEventData eventData) override;

private:
    DISALLOW_COPY_AND_MOVE(BundleEventCallbackImpl);
};

class BundleTestTool : public ShellCommand {
public:
    BundleTestTool(int argc, char *argv[]);
    ~BundleTestTool();

private:
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode Init() override;
    void CreateQuickFixMsgMap(std::unordered_map<int32_t, std::string> &quickFixMsgMap);
    std::string GetResMsg(int32_t code);
    std::string GetResMsg(int32_t code, const std::shared_ptr<QuickFixResult> &quickFixRes);

    ErrCode RunAsHelpCommand();
    ErrCode RunAsCheckCommand();
    ErrCode CheckOperation(int userId, std::string deviceId, std::string bundleName,
        std::string moduleName, std::string abilityName);
    ErrCode RunAsSetRemovableCommand();
    ErrCode RunAsGetRemovableCommand();
    ErrCode RunAsInstallSandboxCommand();
    ErrCode RunAsUninstallSandboxCommand();
    ErrCode RunAsDumpSandboxCommand();
    ErrCode RunAsGetStringCommand();
    ErrCode RunAsGetIconCommand();
    ErrCode RunAsAddInstallRuleCommand();
    ErrCode RunAsGetInstallRuleCommand();
    ErrCode RunAsDeleteInstallRuleCommand();
    ErrCode RunAsCleanInstallRuleCommand();
    ErrCode RunAsAddAppRunningRuleCommand();
    ErrCode RunAsDeleteAppRunningRuleCommand();
    ErrCode RunAsCleanAppRunningRuleCommand();
    ErrCode RunAsGetAppRunningControlRuleCommand();
    ErrCode RunAsGetAppRunningControlRuleResultCommand();
    ErrCode RunAsDeployQuickFix();
    ErrCode RunAsSwitchQuickFix();
    ErrCode RunAsDeleteQuickFix();
    ErrCode RunAsSetDebugMode();
    ErrCode RunAsGetBundleStats();
    ErrCode RunAsGetAppProvisionInfo();
    ErrCode RunAsGetDistributedBundleName();
    ErrCode HandleBundleEventCallback();
    ErrCode ResetAOTCompileStatus();
    ErrCode SendCommonEvent();
    ErrCode RunAsGetProxyDataCommand();
    ErrCode RunAsGetAllProxyDataCommand();
    ErrCode RunAsSetExtNameOrMIMEToAppCommand();
    ErrCode RunAsDelExtNameOrMIMEToAppCommand();
    ErrCode RunAsQueryDataGroupInfos();
    ErrCode RunAsGetGroupDir();
    ErrCode RunAsGetJsonProfile();
    ErrCode RunAsGetUninstalledBundleInfo();
    ErrCode RunAsGetOdid();
    ErrCode CheckImplicitQueryWantOption(int option, std::string &value);
    ErrCode ImplicitQuerySkillUriInfo(const std::string &bundleName,
        const std::string &action, const std::string &entity, const std::string &uri,
        const std::string &type, std::string &msg);
    ErrCode RunAsImplicitQuerySkillUriInfo();
    ErrCode RunAsQueryAbilityInfoByContinueType();

    std::condition_variable cv_;
    std::mutex mutex_;
    bool dataReady_ {false};

    sptr<IBundleMgr> bundleMgrProxy_;
    sptr<IBundleInstaller> bundleInstallerProxy_;
#ifdef DISTRIBUTED_BUNDLE_FRAMEWORK
    sptr<IDistributedBms> distributedBmsProxy_;
#endif

    bool CheckRemovableErrorOption(int option, int counter, const std::string &commandName);
    bool CheckRemovableCorrectOption(int option, const std::string &commandName, int &isRemovable, std::string &name);
    bool SetIsRemovableOperation(const std::string &bundleName, const std::string &moduleName, int isRemovable) const;
    bool GetIsRemovableOperation(
        const std::string &bundleName, const std::string &moduleName, std::string &result) const;
    bool CheckSandboxErrorOption(int option, int counter, const std::string &commandName);
    bool CheckGetStringCorrectOption(int option, const std::string &commandName, int &temp, std::string &name);
    bool CheckGetIconCorrectOption(int option, const std::string &commandName, int &temp, std::string &name);
    ErrCode CheckAddInstallRuleCorrectOption(int option, const std::string &commandName,
        std::vector<std::string> &appIds, int &controlRuleType, int &userId, int &euid);
    ErrCode CheckGetInstallRuleCorrectOption(int option, const std::string &commandName, int &controlRuleType,
        int &userId, int &euid);
    ErrCode CheckDeleteInstallRuleCorrectOption(int option, const std::string &commandName,
        int &controlRuleType, std::vector<std::string> &appIds, int &userId, int &euid);
    ErrCode CheckCleanInstallRuleCorrectOption(int option, const std::string &commandName,
        int &controlRuleType, int &userId, int &euid);
    ErrCode CheckAppRunningRuleCorrectOption(int option, const std::string &commandName,
        std::vector<AppRunningControlRule> &controlRule, int &userId, int &euid);
    ErrCode CheckCleanAppRunningRuleCorrectOption(int option, const std::string &commandName, int &userId, int &euid);
    ErrCode CheckGetAppRunningRuleCorrectOption(int option, const std::string &commandName,
        int32_t &userId, int &euid);
    ErrCode CheckGetAppRunningRuleResultCorrectOption(int option, const std::string &commandName,
        std::string &bundleName, int32_t &userId, int &euid);
    bool CheckSandboxCorrectOption(int option, const std::string &commandName, int &data, std::string &bundleName);
    bool CheckGetProxyDataCorrectOption(int option, const std::string &commandName, int &temp, std::string &name);
    bool CheckGetAllProxyDataCorrectOption(int option, const std::string &commandName, int &temp, std::string &name);
    bool CheckExtOrMimeCorrectOption(int option, const std::string &commandName, int &temp, std::string &name);
    ErrCode InstallSandboxOperation(
        const std::string &bundleName, const int32_t userId, const int32_t dlpType, int32_t &appIndex) const;
    ErrCode UninstallSandboxOperation(
        const std::string &bundleName, const int32_t appIndex, const int32_t userId) const;
    ErrCode DumpSandboxBundleInfo(const std::string &bundleName, const int32_t appIndex, const int32_t userId,
        std::string &dumpResults);
    ErrCode StringToInt(std::string option, const std::string &commandName, int &temp, bool &result);
    ErrCode DeployQuickFix(const std::vector<std::string> &quickFixPaths,
        std::shared_ptr<QuickFixResult> &quickFixRes, bool isDebug);
    ErrCode SwitchQuickFix(const std::string &bundleName, int32_t enable,
        std::shared_ptr<QuickFixResult> &quickFixRes);
    ErrCode DeleteQuickFix(const std::string &bundleName, std::shared_ptr<QuickFixResult> &quickFixRes);
    ErrCode GetQuickFixPath(int32_t index, std::vector<std::string> &quickFixPaths) const;
    ErrCode SetDebugMode(int32_t debugMode);
    bool GetBundleStats(const std::string &bundleName, int32_t userId, std::string &msg);
    ErrCode GetAppProvisionInfo(const std::string &bundleName, int32_t userId, std::string &msg);
    ErrCode GetDistributedBundleName(const std::string &networkId, int32_t accessTokenId, std::string &msg);
    ErrCode BundleNameAndUserIdCommonFunc(std::string &bundleName, int32_t &userId);
    ErrCode CheckGetDistributedBundleNameCorrectOption(int32_t option, const std::string &commandName,
        std::string &networkId, int32_t &accessTokenId);
    bool QueryDataGroupInfos(const std::string &bundleName, int32_t userId, std::string& msg);
    bool ParseEventCallbackOptions(bool &onlyUnregister, int32_t &uid);
    bool ParseResetAOTCompileStatusOptions(std::string &bundleName, std::string &moduleName,
        int32_t &triggerMode, int32_t &uid);
    void Sleep(int32_t seconds);
    ErrCode CallRegisterBundleEventCallback(sptr<BundleEventCallbackImpl> bundleEventCallback);
    ErrCode CallUnRegisterBundleEventCallback(sptr<BundleEventCallbackImpl> bundleEventCallback);
    ErrCode CheckGetGroupIdCorrectOption(int32_t option, std::string &dataGroupId);
    bool GetGroupDir(const std::string &dataGroupId, std::string& msg);
    ErrCode CheckGetBundleNameOption(int32_t option, std::string &bundleName);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_BUNDLEMANAGER_BUNDLE_FRAMEWORK_BUNDLE_TOOL_INCLUDE_BUNDLE_TEST_TOOL_H