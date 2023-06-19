/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "mock_bundle_installer_host.h"

using namespace OHOS::AAFwk;
namespace OHOS {
namespace AppExecFwk {
MockBundleInstallerHost::MockBundleInstallerHost()
{
    APP_LOGI("create mock bundle installer host instance");
}

MockBundleInstallerHost::~MockBundleInstallerHost()
{
    APP_LOGI("destroy mock bundle installer host instance");
}

bool MockBundleInstallerHost::Install(
    const std::string &bundleFilePath, const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver)
{
    APP_LOGI("enter");

    APP_LOGI("bundleFilePath: %{private}s", bundleFilePath.c_str());
    APP_LOGI("installParam.installFlag: %{public}d", installParam.installFlag);

    statusReceiver->OnFinished(OHOS::ERR_OK, MSG_SUCCESS);

    return true;
}

bool MockBundleInstallerHost::Install(const std::vector<std::string> &bundleFilePath, const InstallParam &installParam,
    const sptr<IStatusReceiver> &statusReceiver)
{
    APP_LOGI("enter");

    for_each(bundleFilePath.begin(), bundleFilePath.end(), [](const auto &path)->decltype(auto) {
        APP_LOGI("bundleFilePath: %{private}s", path.c_str());
    });
    APP_LOGI("installParam.installFlag: %{public}d", installParam.installFlag);

    statusReceiver->OnFinished(OHOS::ERR_OK, MSG_SUCCESS);

    return true;
}

bool MockBundleInstallerHost::Uninstall(
    const std::string &bundleName, const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver)
{
    APP_LOGI("enter");

    APP_LOGI("bundleName: %{public}s", bundleName.c_str());
    APP_LOGI("installParam.installFlag: %{public}d", installParam.installFlag);

    statusReceiver->OnFinished(OHOS::ERR_OK, MSG_SUCCESS);

    return true;
}

bool MockBundleInstallerHost::Uninstall(const std::string &bundleName, const std::string &modulePackage,
    const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver)
{
    APP_LOGI("enter");

    APP_LOGI("bundleName: %{public}s", bundleName.c_str());
    APP_LOGI("modulePackage: %{public}s", modulePackage.c_str());
    APP_LOGI("installParam.installFlag: %{public}d", installParam.installFlag);

    statusReceiver->OnFinished(OHOS::ERR_OK, MSG_SUCCESS);

    return true;
}

bool MockBundleInstallerHost::Recover(const std::string &bundleName, const InstallParam &installParam,
    const sptr<IStatusReceiver> &statusReceiver)
{
    APP_LOGD("enter");
    APP_LOGD("bundleName: %{public}s", bundleName.c_str());
    APP_LOGD("installParam.installFlag: %{public}d", installParam.installFlag);
    statusReceiver->OnFinished(OHOS::ERR_OK, MSG_SUCCESS);
    return true;
}

ErrCode MockBundleInstallerHost::StreamInstall(const std::vector<std::string> &bundleFilePaths,
    const InstallParam &installParam, const sptr<IStatusReceiver> &statusReceiver)
{
    APP_LOGD("enter");
    statusReceiver->OnFinished(OHOS::ERR_OK, MSG_SUCCESS);
    return OHOS::ERR_OK;
}

ErrCode MockBundleInstallerHost::InstallSandboxApp(const std::string &bundleName, int32_t dplType, int32_t userId,
    int32_t &appIndex)
{
    return OHOS::ERR_OK;
}

ErrCode MockBundleInstallerHost::UninstallSandboxApp(const std::string &bundleName, int32_t appIndex, int32_t userId)
{
    return OHOS::ERR_OK;
}

sptr<IBundleStreamInstaller> MockBundleInstallerHost::CreateStreamInstaller(const InstallParam &installParam,
    const sptr<IStatusReceiver> &statusReceiver)
{
    return nullptr;
}

bool MockBundleInstallerHost::DestoryBundleStreamInstaller(uint32_t streamInstallerId)
{
    return true;
}

bool MockBundleInstallerHost::Uninstall(const UninstallParam &uninstallParam,
    const sptr<IStatusReceiver> &statusReceiver)
{
    statusReceiver->OnFinished(OHOS::ERR_OK, MSG_SUCCESS);
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS