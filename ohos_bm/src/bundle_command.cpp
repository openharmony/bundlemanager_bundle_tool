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
#include "bundle_command.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <future>
#include <getopt.h>
#include <unistd.h>
#include <vector>
#include "app_log_wrapper.h"
#include "app_mgr_client.h"
#include "bundle_command_common.h"
#include "bundle_death_recipient.h"
#include "bundle_mgr_client.h"
#include "bundle_mgr_proxy.h"
#include "clean_cache_callback_host.h"
#include "error_code_utils.h"
#include "ipc_skeleton.h"
#include "json_serializer.h"
#include "nlohmann/json.hpp"
#include "status_receiver_impl.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const char* BMS_PARA_INSTALL_ALLOW_DOWNGRADE = "ohos.bms.param.installAllowDowngrade";
const char* BMS_PARA_INSTALL_GRANT_PERMISSION = "ohos.bms.param.installAddPermission";
const int32_t INDEX_OFFSET = 2;
const int32_t MINIMUM_WAITTING_TIME = 180; // 3 mins
const int32_t MAXIMUM_WAITTING_TIME = 600; // 10 mins

const std::string SHORT_OPTIONS = "hp:rn:m:a:cdu:w:s:i:g";
const struct option LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundlePath", required_argument, nullptr, 'p'},
    {"replace", no_argument, nullptr, 'r'},
    {"bundleName", required_argument, nullptr, 'n'},
    {"moduleName", required_argument, nullptr, 'm'},
    {"abilityName", required_argument, nullptr, 'a'},
    {"bundleInfo", no_argument, nullptr, 'i'},
    {"cache", no_argument, nullptr, 'c'},
    {"downgrade", no_argument, nullptr, 'd'},
    {"isRemovable", required_argument, nullptr, 'i'},
    {"userId", required_argument, nullptr, 'u'},
    {"waittingTime", required_argument, nullptr, 'w'},
    {"keepData", no_argument, nullptr, 'k'},
    {"sharedBundleDirPath", required_argument, nullptr, 's'},
    {"appIndex", required_argument, nullptr, 'i'},
    {"grantPermission", no_argument, nullptr, 'g'},
    {nullptr, 0, nullptr, 0},
};

const std::string UNINSTALL_OPTIONS = "hn:km:u:v:s";
const struct option UNINSTALL_LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundleName", required_argument, nullptr, 'n'},
    {"moduleName", required_argument, nullptr, 'm'},
    {"userId", required_argument, nullptr, 'u'},
    {"keepData", no_argument, nullptr, 'k'},
    {"version", required_argument, nullptr, 'v'},
    {"shared", no_argument, nullptr, 's'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_DUMP = "hn:aisu:d:gl";
const struct option LONG_OPTIONS_DUMP[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundleName", required_argument, nullptr, 'n'},
    {"all", no_argument, nullptr, 'a'},
    {"bundleInfo", no_argument, nullptr, 'i'},
    {"shortcutInfo", no_argument, nullptr, 's'},
    {"userId", required_argument, nullptr, 'u'},
    {"deviceId", required_argument, nullptr, 'd'},
    {"debugBundle", no_argument, nullptr, 'g'},
    {"label", no_argument, nullptr, 'l'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_DUMP_SHARED_DEPENDENCIES = "hn:m:";
const struct option LONG_OPTIONS_DUMP_SHARED_DEPENDENCIES[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundleName", required_argument, nullptr, 'n'},
    {"moduleName", required_argument, nullptr, 'm'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_DUMP_SHARED = "hn:a";
const struct option LONG_OPTIONS_DUMP_SHARED[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundleName", required_argument, nullptr, 'n'},
    {"all", no_argument, nullptr, 'a'},
    {nullptr, 0, nullptr, 0},
};

const std::string CLEAN_SHORT_OPTIONS = "hn:cdu:i:";
const struct option CLEAN_LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundleName", required_argument, nullptr, 'n'},
    {"cache", no_argument, nullptr, 'c'},
    {"data", no_argument, nullptr, 'd'},
    {"userId", required_argument, nullptr, 'u'},
    {"appIndex", required_argument, nullptr, 'i'},
    {nullptr, 0, nullptr, 0},
};

class CleanCacheCallbackImpl : public CleanCacheCallbackHost {
public:
    CleanCacheCallbackImpl() : signal_(std::make_shared<std::promise<bool>>())
    {}
    ~CleanCacheCallbackImpl() override
    {}
    void OnCleanCacheFinished(bool error) override;
    bool GetResultCode();
private:
    std::shared_ptr<std::promise<bool>> signal_;
    DISALLOW_COPY_AND_MOVE(CleanCacheCallbackImpl);
};

void CleanCacheCallbackImpl::OnCleanCacheFinished(bool error)
{
    if (signal_ != nullptr) {
        signal_->set_value(error);
    }
}

bool CleanCacheCallbackImpl::GetResultCode()
{
    if (signal_ != nullptr) {
        auto future = signal_->get_future();
        std::chrono::milliseconds span(MAX_WAITING_TIME);
        if (future.wait_for(span) == std::future_status::timeout) {
            return false;
        }
        return future.get();
    }
    return false;
}
}  // namespace

BundleManagerShellCommand::BundleManagerShellCommand(int argc, char *argv[])
    : ShellCommand(argc, argv, TOOL_NAME)
{}

std::string BundleManagerShellCommand::CreateSuccessResult(const std::string &message,
    const std::string &data) const
{
    nlohmann::json result;
    result["type"] = "result";
    result["status"] = "success";
    if (data.empty()) {
        result["data"] = nlohmann::json::object();
    } else if (nlohmann::json::accept(data)) {
        result["data"] = nlohmann::json::parse(data);
    } else {
        // If data is not valid JSON, wrap it as a string in an object
        result["data"] = nlohmann::json::object();
        result["data"]["content"] = data;
    }
    result["errCode"] = "SUCCESS";
    result["errMsg"] = message;
    return result.dump();
}

std::string BundleManagerShellCommand::CreateErrorResult(int32_t code,
    const std::string &message, const std::string &suggestion) const
{
    nlohmann::json result;
    result["type"] = "result";
    result["status"] = "failed";
    result["data"] = nlohmann::json::object();
    result["errCode"] = ErrorCodeUtils::GetErrorCodeString(code);
    std::string errMsg = message;
    std::string codeMessage = GetMessageFromCode(code);
    if (!codeMessage.empty()) {
        errMsg += "\n" + codeMessage;
    }
    result["errMsg"] = errMsg;
    if (!suggestion.empty()) {
        result["suggestion"] = suggestion;
    }
    return result.dump();
}

std::string BundleManagerShellCommand::CreateErrorResult(const std::string &errCode,
    const std::string &message, const std::string &suggestion) const
{
    nlohmann::json result;
    result["type"] = "result";
    result["status"] = "failed";
    result["data"] = nlohmann::json::object();
    result["errCode"] = errCode;
    result["errMsg"] = message;
    if (!suggestion.empty()) {
        result["suggestion"] = suggestion;
    }
    return result.dump();
}

ErrCode BundleManagerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", [this] { return this->RunAsHelpCommand(); } },
        {"install", [this] { return this->RunAsInstallCommand(); } },
        {"uninstall", [this] { return this->RunAsUninstallCommand(); } },
        {"dump", [this] { return this->RunAsDumpCommand(); } },
        {"dump-dependencies", [this] { return this->RunAsDumpSharedDependenciesCommand(); } },
        {"dump-shared", [this] { return this->RunAsDumpSharedCommand(); } },
        {"clean", [this] { return this->RunAsCleanCommand(); } },
        {"set-disposed-rule", [this] { return this->RunAsSetDisposedRuleCommand(); } },
        {"delete-disposed-rule", [this] { return this->RunAsDeleteDisposedRuleCommand(); } },
    };
    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::CreateMessageMap()
{
    messageMap_ = BundleCommandCommon::bundleMessageMap_;
    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::Init()
{
    ErrCode result = OHOS::ERR_OK;

    if (bundleMgrProxy_ == nullptr) {
        bundleMgrProxy_ = BundleCommandCommon::GetBundleMgrProxy();
    }

    if (bundleMgrProxy_ == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode BundleManagerShellCommand::InitAppControlProxy()
{
    ErrCode result = OHOS::ERR_OK;
    if (appControlProxy_ == nullptr && bundleMgrProxy_ != nullptr) {
        appControlProxy_ = bundleMgrProxy_->GetAppControlProxy();
    }
    if (appControlProxy_ == nullptr || appControlProxy_->AsObject() == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
    }
    return result;
}

ErrCode BundleManagerShellCommand::InitInstaller()
{
    ErrCode result = OHOS::ERR_OK;

    if (bundleInstallerProxy_ == nullptr && bundleMgrProxy_ != nullptr) {
        bundleInstallerProxy_ = bundleMgrProxy_->GetBundleInstaller();
    }

    if (bundleInstallerProxy_ == nullptr || bundleInstallerProxy_->AsObject() == nullptr) {
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode BundleManagerShellCommand::RunAsHelpCommand()
{
    resultReceiver_ = CreateSuccessResult(HELP_MSG);
    return OHOS::ERR_OK;
}

bool BundleManagerShellCommand::IsInstallOption(int index) const
{
    if (index >= argc_ || index < INDEX_OFFSET) {
        return false;
    }
    if (argList_[index - INDEX_OFFSET] == "-r" || argList_[index - INDEX_OFFSET] == "--replace" ||
        argList_[index - INDEX_OFFSET] == "-p" || argList_[index - INDEX_OFFSET] == "--bundlePath" ||
        argList_[index - INDEX_OFFSET] == "-u" || argList_[index - INDEX_OFFSET] == "--userId" ||
        argList_[index - INDEX_OFFSET] == "-w" || argList_[index - INDEX_OFFSET] == "--waittingTime" ||
        argList_[index - INDEX_OFFSET] == "-s" || argList_[index - INDEX_OFFSET] == "--sharedBundleDirPath" ||
        argList_[index - INDEX_OFFSET] == "-d" || argList_[index - INDEX_OFFSET] == "--downgrade" ||
        argList_[index - INDEX_OFFSET] == "-g" || argList_[index - INDEX_OFFSET] == "--add-permission") {
        return true;
    }
    return false;
}

ErrCode BundleManagerShellCommand::RunAsInstallCommand()
{
    APP_LOGI("begin to RunAsInstallCommand");

    // Return error when no arguments provided
    if (argc_ <= 2) {
        APP_LOGD("'ohos-bm install' with no option.");
        resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    // install 命令需要 installer proxy
    if (InitInstaller() != OHOS::ERR_OK) {
        resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_INTERNAL_ERROR,
            "error: failed to connect to bundle installer service.");
        return OHOS::ERR_INVALID_VALUE;
    }

    int result = OHOS::ERR_OK;
    InstallFlag installFlag = InstallFlag::REPLACE_EXISTING;
    int counter = 0;
    std::vector<std::string> bundlePath;
    std::vector<std::string> sharedBundleDirPaths;
    int index = 0;
    int hspIndex = 0;
    const int32_t currentUser = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    int32_t userId = currentUser;
    int32_t waittingTime = MINIMUM_WAITTING_TIME;
    std::string warning;
    bool isDowngrade = false;
    bool grantPermission = false;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (option == -1) {
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'p': {
                    APP_LOGD("'ohos-bm install' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    APP_LOGD("'ohos-bm install -u' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'w': {
                    APP_LOGD("'ohos-bm install -w' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'ohos-bm install' with an unknown option.");
                    resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm install %{public}s'", argv_[optind - 1]);
                resultReceiver_ = CreateSuccessResult(HELP_MSG_INSTALL);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'p': {
                APP_LOGD("'ohos-bm install %{public}s'", argv_[optind - 1]);
                if (GetBundlePath(optarg, bundlePath) != OHOS::ERR_OK) {
                    APP_LOGD("'ohos-bm install' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                index = optind;
                break;
            }
            case 'r': {
                installFlag = InstallFlag::REPLACE_EXISTING;
                break;
            }
            case 'u': {
                APP_LOGW("'ohos-bm install -u only support user 0'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("ohos-bm install with error userId %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != Constants::DEFAULT_USERID && !BundleCommandCommon::IsUserForeground(userId)) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
                }
                break;
            }
            case 'w': {
                APP_LOGD("'ohos-bm install %{public}s %{public}s'", argv_[optind - INDEX_OFFSET], optarg);
                if (!OHOS::StrToInt(optarg, waittingTime) || waittingTime < MINIMUM_WAITTING_TIME ||
                    waittingTime > MAXIMUM_WAITTING_TIME) {
                    APP_LOGE("ohos-bm install with error waittingTime %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            case 's': {
                APP_LOGD("'ohos-bm install %{public}s %{public}s'", argv_[optind - INDEX_OFFSET], optarg);
                if (GetBundlePath(optarg, sharedBundleDirPaths) != OHOS::ERR_OK) {
                    APP_LOGD("'ohos-bm install -s' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                hspIndex = optind;
                break;
            }
            case 'd': {
                isDowngrade = true;
                break;
            }
            case 'g': {
                grantPermission = true;
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    for (; index < argc_ && index >= INDEX_OFFSET; ++index) {
        if (IsInstallOption(index)) {
            break;
        }
        if (GetBundlePath(argList_[index - INDEX_OFFSET], bundlePath) != OHOS::ERR_OK) {
            bundlePath.clear();
            APP_LOGD("'ohos-bm install' with error arguments.");
            resultReceiver_ = CreateErrorResult(
                IStatusReceiver::ERR_INSTALL_PARAM_ERROR, "error value for the chosen option");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    // hsp list
    for (; hspIndex < argc_ && hspIndex >= INDEX_OFFSET; ++hspIndex) {
        if (IsInstallOption(hspIndex)) {
            break;
        }
        if (GetBundlePath(argList_[hspIndex - INDEX_OFFSET], sharedBundleDirPaths) != OHOS::ERR_OK) {
            sharedBundleDirPaths.clear();
            APP_LOGD("'ohos-bm install -s' with error arguments.");
            resultReceiver_ = CreateErrorResult(
                IStatusReceiver::ERR_INSTALL_PARAM_ERROR, "error value for the chosen option");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    for (auto &path : bundlePath) {
        APP_LOGD("install hap path %{private}s", path.c_str());
    }

    for (auto &path : sharedBundleDirPaths) {
        APP_LOGD("install hsp path %{private}s", path.c_str());
    }

    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && bundlePath.empty() && sharedBundleDirPaths.empty()) {
            APP_LOGD("'ohos-bm install' with no bundle path option.");
            resultReceiver_ = CreateErrorResult(
                IStatusReceiver::ERR_INSTALL_FILE_PATH_INVALID, HELP_MSG_NO_BUNDLE_PATH_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        if (resultReceiver_ == "") {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_INSTALL);
        }
    } else {
        InstallParam installParam;
        installParam.installFlag = installFlag;
        installParam.userId = userId;
        installParam.sharedBundleDirPaths = sharedBundleDirPaths;
        if (isDowngrade) {
            APP_LOGI("install allow downgrade");
            installParam.parameters[BMS_PARA_INSTALL_ALLOW_DOWNGRADE] = "true";
        }
        if (grantPermission) {
            APP_LOGI("install allow grantPermission");
            installParam.parameters[BMS_PARA_INSTALL_GRANT_PERMISSION] = "true";
        }
        std::string resultMsg;
        int32_t installResult = InstallOperation(bundlePath, installParam, waittingTime, resultMsg);
        if (installResult == OHOS::ERR_OK) {
            resultReceiver_ = CreateSuccessResult(STRING_INSTALL_BUNDLE_OK, resultMsg);
        } else {
            resultReceiver_ = CreateErrorResult(installResult, STRING_INSTALL_BUNDLE_NG);
        }
        if (!warning.empty()) {
            nlohmann::json resultJson = nlohmann::json::parse(resultReceiver_);
            resultJson["warning"] = warning;
            resultReceiver_ = resultJson.dump();
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::GetBundlePath(const std::string& param,
    std::vector<std::string>& bundlePaths) const
{
    if (param.empty()) {
        return OHOS::ERR_INVALID_VALUE;
    }
    if (param == "-r" || param == "--replace" || param == "-p" ||
        param == "--bundlePath" || param == "-u" || param == "--userId" ||
        param == "-w" || param == "--waittingTime") {
        return OHOS::ERR_INVALID_VALUE;
    }
    bundlePaths.emplace_back(param);
    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::RunAsUninstallCommand()
{
    APP_LOGI("begin to RunAsUninstallCommand");

    // Return error when no arguments provided
    if (argc_ <= 2) {
        APP_LOGD("'ohos-bm uninstall' with no option.");
        resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    // uninstall 命令需要 installer proxy
    if (InitInstaller() != OHOS::ERR_OK) {
        resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_INTERNAL_ERROR,
            "error: failed to connect to bundle installer service.");
        return OHOS::ERR_INVALID_VALUE;
    }

    int result = OHOS::ERR_OK;
    int counter = 0;
    std::string bundleName = "";
    std::string moduleName = "";
    const int32_t currentUser = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    std::string warning;
    int32_t userId = currentUser;
    bool isKeepData = false;
    bool isShared = false;
    int32_t versionCode = Constants::ALL_VERSIONCODE;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, UNINSTALL_OPTIONS.c_str(), UNINSTALL_LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (option == -1) {
            break;
        }

        if (option == '?') {
                resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_OPTION);
                result = OHOS::ERR_INVALID_VALUE;
            }

        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    APP_LOGD("'ohos-bm uninstall -n' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'm': {
                    APP_LOGD("'ohos-bm uninstall -m' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    APP_LOGD("'ohos-bm uninstall -u' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'k': {
                    isKeepData = true;
                    break;
                }
                case 's': {
                    isShared = true;
                    break;
                }
                case 'v': {
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'ohos-bm uninstall' with an unknown option.");
                    resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm uninstall %{public}s'", argv_[optind - 1]);
                resultReceiver_ = CreateSuccessResult(HELP_MSG_UNINSTALL);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                APP_LOGD("'ohos-bm uninstall %{public}s %{public}s'",
                    argv_[optind - INDEX_OFFSET], optarg);
                bundleName = optarg;
                break;
            }
            case 'm': {
                APP_LOGD("'ohos-bm uninstall %{public}s %{public}s'",
                    argv_[optind - INDEX_OFFSET], optarg);
                moduleName = optarg;
                break;
            }
            case 'u': {
                APP_LOGW("'ohos-bm uninstall -u only support user 0'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("ohos-bm uninstall with error userId %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != Constants::DEFAULT_USERID && !BundleCommandCommon::IsUserForeground(userId)) {
                    warning = GetWaringString(currentUser, userId);
                    userId = currentUser;
                }
                break;
            }
            case 'k': {
                APP_LOGD("'ohos-bm uninstall %{public}s'", argv_[optind - INDEX_OFFSET]);
                isKeepData = true;
                break;
            }
            case 's': {
                APP_LOGD("'ohos-bm uninstall -s'");
                isShared = true;
                break;
            }
            case 'v': {
                APP_LOGD("'ohos-bm uninstall %{public}s %{public}s'",
                    argv_[optind - INDEX_OFFSET], optarg);
                if (!OHOS::StrToInt(optarg, versionCode) || versionCode < 0) {
                    APP_LOGE("ohos-bm uninstall with error versionCode %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && bundleName.size() == 0) {
            APP_LOGD("'ohos-bm uninstall' with bundle name option.");
            resultReceiver_ = CreateErrorResult(
                IStatusReceiver::ERR_UNINSTALL_INVALID_NAME, HELP_MSG_NO_BUNDLE_NAME_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        if (resultReceiver_ == "") {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_UNINSTALL);
        }
        return result;
    }

    if (isShared) {
        UninstallParam uninstallParam;
        uninstallParam.bundleName = bundleName;
        uninstallParam.versionCode = versionCode;
        APP_LOGE("version code is %{public}d", versionCode);
        int32_t uninstallResult = UninstallSharedOperation(uninstallParam);
        if (uninstallResult == OHOS::ERR_OK) {
            resultReceiver_ = CreateSuccessResult(STRING_UNINSTALL_BUNDLE_OK);
        } else {
            resultReceiver_ = CreateErrorResult(uninstallResult,
                STRING_UNINSTALL_BUNDLE_NG);
        }
    } else {
        InstallParam installParam;
        installParam.userId = userId;
        installParam.isKeepData = isKeepData;
        installParam.parameters.emplace(Constants::VERIFY_UNINSTALL_RULE_KEY,
            Constants::VERIFY_UNINSTALL_RULE_VALUE);
        int32_t uninstallResult = UninstallOperation(bundleName, moduleName, installParam);
        if (uninstallResult == OHOS::ERR_OK) {
            resultReceiver_ = CreateSuccessResult(STRING_UNINSTALL_BUNDLE_OK);
        } else {
            resultReceiver_ = CreateErrorResult(uninstallResult,
                STRING_UNINSTALL_BUNDLE_NG);
        }
        if (!warning.empty()) {
            nlohmann::json resultJson = nlohmann::json::parse(resultReceiver_);
            resultJson["warning"] = warning;
            resultReceiver_ = resultJson.dump();
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsDumpCommand()
{
    APP_LOGI("begin to RunAsDumpCommand");
    int result = OHOS::ERR_OK;

    // Return error when no arguments provided
    if (argc_ <= 2) {
        APP_LOGD("'ohos-bm dump' with no option.");
        resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    int counter = 0;
    std::string bundleName = "";
    bool bundleDumpAll = false;
    bool bundleDumpDebug = false;
    bool bundleDumpInfo = false;
    bool bundleDumpShortcut = false;
    bool bundleDumpDistributedBundleInfo = false;
    bool bundleDumpLabel = false;
    std::string deviceId = "";
    const int32_t currentUser = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    int32_t userId = currentUser;
    std::string warning;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS_DUMP.c_str(), LONG_OPTIONS_DUMP, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (option == -1) {
            break;
        }
        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    APP_LOGD("'ohos-bm dump -n' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    APP_LOGD("'ohos-bm dump -u' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'd': {
                    APP_LOGD("'ohos-bm dump -d' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'ohos-bm dump' with an unknown option.");
                    resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm dump %{public}s'", argv_[optind - 1]);
                resultReceiver_ = CreateSuccessResult(HELP_MSG_DUMP);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'a': {
                APP_LOGD("'ohos-bm dump %{public}s'", argv_[optind - 1]);
                bundleDumpAll = true;
                break;
            }
            case 'l': {
                APP_LOGD("'ohos-bm dump %{public}s'", argv_[optind - 1]);
                bundleDumpLabel = true;
                break;
            }
            case 'g': {
                APP_LOGD("'ohos-bm dump %{public}s'", argv_[optind - 1]);
                bundleDumpDebug = true;
                break;
            }
            case 'n': {
                APP_LOGD("'ohos-bm dump %{public}s %{public}s'", argv_[optind - INDEX_OFFSET], optarg);
                bundleName = optarg;
                bundleDumpInfo = true;
                break;
            }
            case 's': {
                APP_LOGD("'ohos-bm dump %{public}s %{public}s'", argv_[optind - INDEX_OFFSET], optarg);
                bundleDumpShortcut = true;
                break;
            }
            case 'u': {
                APP_LOGW("'ohos-bm dump -u is not supported'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("ohos-bm dump with error userId %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != Constants::DEFAULT_USERID && !BundleCommandCommon::IsUserForeground(userId)) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
                }
                break;
            }
            case 'd': {
                APP_LOGD("'ohos-bm dump %{public}s %{public}s'", argv_[optind - INDEX_OFFSET], optarg);
                deviceId = optarg;
                bundleDumpDistributedBundleInfo = true;
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    if (result == OHOS::ERR_OK) {
        if ((resultReceiver_ == "") && bundleDumpShortcut && (bundleName.size() == 0)) {
            APP_LOGD("'ohos-bm dump -s' with no bundle name option.");
            resultReceiver_ = CreateErrorResult(
                IStatusReceiver::ERR_UNINSTALL_INVALID_NAME, HELP_MSG_NO_BUNDLE_NAME_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
        if ((resultReceiver_ == "") && bundleDumpDistributedBundleInfo && (bundleName.size() == 0)) {
            APP_LOGD("'ohos-bm dump -d' with no bundle name option.");
            resultReceiver_ = CreateErrorResult(
                IStatusReceiver::ERR_UNINSTALL_INVALID_NAME, HELP_MSG_NO_BUNDLE_NAME_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        if (resultReceiver_ == "") {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_DUMP);
        }
    } else {
        std::string dumpResults = "";
        if (bundleDumpShortcut) {
            dumpResults = DumpShortcutInfos(bundleName, userId);
        } else if (bundleDumpDistributedBundleInfo) {
            dumpResults = DumpDistributedBundleInfo(deviceId, bundleName);
        } else if (bundleDumpAll && !bundleDumpLabel) {
            dumpResults = DumpBundleList(userId);
        } else if (bundleDumpDebug) {
            dumpResults = DumpDebugBundleList(userId);
        } else if (bundleDumpInfo && !bundleDumpLabel) {
            dumpResults = DumpBundleInfo(bundleName, userId);
        } else if (bundleDumpAll && bundleDumpLabel) {
            dumpResults = DumpAllLabel(userId);
        } else if (bundleDumpInfo && bundleDumpLabel) {
            dumpResults = DumpBundleLabel(bundleName, userId);
        }
        if (dumpResults.empty()) {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARSE_FAILED, HELP_MSG_DUMP_FAILED);
        } else {
            resultReceiver_ = CreateSuccessResult("dump successfully", dumpResults);
        }
        if (!warning.empty()) {
            nlohmann::json resultJson = nlohmann::json::parse(resultReceiver_);
            resultJson["warning"] = warning;
            resultReceiver_ = resultJson.dump();
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsDumpSharedDependenciesCommand()
{
    APP_LOGI("begin to RunAsDumpSharedDependenciesCommand");
    int32_t result = OHOS::ERR_OK;

    // Return error when no arguments provided
    if (argc_ <= 2) {
        resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    int32_t counter = 0;
    std::string bundleName;
    std::string moduleName;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS_DUMP_SHARED_DEPENDENCIES.c_str(),
            LONG_OPTIONS_DUMP_SHARED_DEPENDENCIES, nullptr);
        if (option == -1) {
            break;
        }
        result = ParseSharedDependenciesCommand(option, bundleName, moduleName);
        if (option == '?') {
            break;
        }
    }
    if (result == OHOS::ERR_OK) {
        if ((resultReceiver_ == "") && (bundleName.size() == 0 || moduleName.size() == 0)) {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_REMOVABLE_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        if (resultReceiver_ == "") {
            resultReceiver_ = CreateErrorResult(
                IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_DUMP_SHARED_DEPENDENCIES);
        }
    } else {
        std::string dumpResults = DumpSharedDependencies(bundleName, moduleName);
        if (dumpResults.empty()) {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARSE_FAILED, HELP_MSG_DUMP_FAILED);
        } else {
            resultReceiver_ = CreateSuccessResult("dump dependencies successfully", dumpResults);
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::ParseSharedDependenciesCommand(int32_t option, std::string &bundleName,
    std::string &moduleName)
{
    int32_t result = OHOS::ERR_OK;
    if (option == '?') {
        switch (optopt) {
            case 'n': {
                resultReceiver_ = CreateErrorResult(
                    IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'm': {
                resultReceiver_ = CreateErrorResult(
                    IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    } else {
        switch (option) {
            case 'h': {
                resultReceiver_ = CreateSuccessResult(HELP_MSG_DUMP_SHARED_DEPENDENCIES);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                bundleName = optarg;
                break;
            }
            case 'm': {
                moduleName = optarg;
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    return result;
}

ErrCode BundleManagerShellCommand::RunAsDumpSharedCommand()
{
    APP_LOGI("begin to RunAsDumpSharedCommand");
    int32_t result = OHOS::ERR_OK;

    // Return error when no arguments provided
    if (argc_ <= 2) {
        resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    int32_t counter = 0;
    std::string bundleName;
    bool dumpSharedAll = false;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS_DUMP_SHARED.c_str(),
            LONG_OPTIONS_DUMP_SHARED, nullptr);
        if (option == -1) {
            break;
        }
        result = ParseSharedCommand(option, bundleName, dumpSharedAll);
        if (option == '?') {
            break;
        }
    }
    if (result != OHOS::ERR_OK) {
        if (resultReceiver_ == "") {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_DUMP_SHARED);
        }
    } else if (dumpSharedAll) {
        std::string dumpResults = DumpSharedAll();
        resultReceiver_ = CreateSuccessResult("dump shared all successfully", dumpResults);
    } else {
        if ((resultReceiver_ == "") && (bundleName.size() == 0)) {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_REMOVABLE_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
            return result;
        }
        std::string dumpResults = DumpShared(bundleName);
        if (dumpResults.empty()) {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARSE_FAILED, HELP_MSG_DUMP_FAILED);
        } else {
            resultReceiver_ = CreateSuccessResult("dump shared successfully", dumpResults);
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::ParseSharedCommand(int32_t option, std::string &bundleName, bool &dumpSharedAll)
{
    int32_t result = OHOS::ERR_OK;
    if (option == '?') {
        switch (optopt) {
            case 'n': {
                resultReceiver_ = CreateErrorResult(
                    IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    } else {
        switch (option) {
            case 'h': {
                resultReceiver_ = CreateSuccessResult(HELP_MSG_DUMP_SHARED);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                bundleName = optarg;
                break;
            }
            case 'a': {
                dumpSharedAll = true;
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    return result;
}

ErrCode BundleManagerShellCommand::RunAsCleanCommand()
{
    APP_LOGI("begin to RunAsCleanCommand");
    int32_t result = OHOS::ERR_OK;

    // Return error when no arguments provided
    if (argc_ <= 2) {
        APP_LOGD("'ohos-bm clean' with no option.");
        resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    int32_t counter = 0;
    const int32_t currentUser = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    int32_t userId = currentUser;
    std::string warning;
    int32_t appIndex = 0;
    bool cleanCache = false;
    bool cleanData = false;
    std::string bundleName = "";
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, CLEAN_SHORT_OPTIONS.c_str(), CLEAN_LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (option == -1) {
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    APP_LOGD("'ohos-bm clean -n' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    APP_LOGD("'ohos-bm clean -u' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'i': {
                    APP_LOGD("'ohos-bm clean -i' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'ohos-bm clean' with an unknown option.");
                    resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm clean %{public}s'", argv_[optind - 1]);
                resultReceiver_ = CreateSuccessResult(HELP_MSG_CLEAN);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                APP_LOGD("'ohos-bm clean %{public}s %{public}s'", argv_[optind - INDEX_OFFSET], optarg);
                bundleName = optarg;
                break;
            }
            case 'c': {
                APP_LOGD("'ohos-bm clean %{public}s'", argv_[optind - INDEX_OFFSET]);
                cleanCache = cleanData ? false : true;
                break;
            }
            case 'd': {
                APP_LOGD("'ohos-bm clean %{public}s '", argv_[optind - INDEX_OFFSET]);
                cleanData = cleanCache ? false : true;
                break;
            }
            case 'u': {
                APP_LOGW("'ohos-bm clean -u is not supported'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("ohos-bm clean with error userId %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (!BundleCommandCommon::IsUserForeground(userId)) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
                }
                break;
            }
            case 'i': {
                if (!OHOS::StrToInt(optarg, appIndex) || (appIndex < 0 || appIndex > INITIAL_SANDBOX_APP_INDEX)) {
                    APP_LOGE("ohos-bm clean with error appIndex %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_INSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && bundleName.size() == 0) {
            APP_LOGD("'ohos-bm clean' with no bundle name option.");
            resultReceiver_ = CreateErrorResult(
                IStatusReceiver::ERR_UNINSTALL_INVALID_NAME, HELP_MSG_NO_BUNDLE_NAME_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
        if (!cleanCache && !cleanData) {
            APP_LOGD("'ohos-bm clean' with no '-c' or '-d' option.");
            resultReceiver_ = CreateErrorResult(
                IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_NO_DATA_OR_CACHE_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        if (resultReceiver_ == "") {
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_INSTALL_PARAM_ERROR, HELP_MSG_CLEAN);
        }
    } else {
        nlohmann::json resultJson;
        resultJson["status"] = true;
        resultJson["data"] = "";
        resultJson["error_code"] = IStatusReceiver::SUCCESS;
        resultJson["error_message"] = "";
        nlohmann::json cleanResult;
        if (cleanCache) {
            if (CleanBundleCacheFilesOperation(bundleName, userId, appIndex)) {
                cleanResult["cache"] = STRING_CLEAN_CACHE_BUNDLE_OK;
            } else {
                resultJson["status"] = false;
                resultJson["error_code"] = IStatusReceiver::ERR_INSTALL_PARSE_FAILED;
                cleanResult["cache"] = STRING_CLEAN_CACHE_BUNDLE_NG;
            }
        }
        if (cleanData) {
            if (CleanBundleDataFilesOperation(bundleName, userId, appIndex)) {
                cleanResult["data"] = STRING_CLEAN_DATA_BUNDLE_OK;
            } else {
                resultJson["status"] = false;
                resultJson["error_code"] = IStatusReceiver::ERR_INSTALL_PARSE_FAILED;
                cleanResult["data"] = STRING_CLEAN_DATA_BUNDLE_NG;
            }
        }
        resultJson["data"] = cleanResult.dump();
        if (!warning.empty()) {
            resultJson["warning"] = warning;
        }
        resultReceiver_ = resultJson.dump();
    }
    APP_LOGI("end");
    return result;
}

// Dump helper methods

std::string BundleManagerShellCommand::DumpBundleList(int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_BUNDLE_LIST, BUNDLE_NAME_EMPTY, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump bundle list.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpBundleLabel(const std::string &bundleName, int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_BUNDLE_LABEL, bundleName, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump bundle label.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpAllLabel(int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_LABEL_LIST, BUNDLE_NAME_EMPTY, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump bundle label list.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpDebugBundleList(int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_DEBUG_BUNDLE_LIST, BUNDLE_NAME_EMPTY, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump debug bundle list.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpBundleInfo(const std::string &bundleName, int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_BUNDLE_INFO, bundleName, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump bundle info.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpShortcutInfos(const std::string &bundleName, int32_t userId) const
{
    std::string dumpResults;
    bool dumpRet = bundleMgrProxy_->DumpInfos(
        DumpFlag::DUMP_SHORTCUT_INFO, bundleName, userId, dumpResults);
    if (!dumpRet) {
        APP_LOGE("failed to dump shortcut infos.");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpDistributedBundleInfo(
    const std::string &deviceId, const std::string &bundleName)
{
    std::string dumpResults = "";
    DistributedBundleInfo distributedBundleInfo;
    bool dumpRet = bundleMgrProxy_->GetDistributedBundleInfo(deviceId, bundleName, distributedBundleInfo);
    if (!dumpRet) {
        APP_LOGE("failed to dump distributed bundleInfo.");
    } else {
        dumpResults.append("distributed bundleInfo");
        dumpResults.append(":\n");
        dumpResults.append(distributedBundleInfo.ToString());
        dumpResults.append("\n");
    }
    return dumpResults;
}

std::string BundleManagerShellCommand::DumpSharedDependencies(const std::string &bundleName,
    const std::string &moduleName) const
{
    APP_LOGD("DumpSharedDependencies bundleName: %{public}s, moduleName: %{public}s",
        bundleName.c_str(), moduleName.c_str());
    std::vector<Dependency> dependencies;
    ErrCode ret = bundleMgrProxy_->GetSharedDependencies(bundleName, moduleName, dependencies);
    nlohmann::json dependenciesJson;
    if (ret != ERR_OK) {
        APP_LOGE("dump shared dependencies failed due to errcode %{public}d", ret);
        return std::string();
    } else {
        dependenciesJson = nlohmann::json {{DEPENDENCIES, dependencies}};
    }
    return dependenciesJson.dump(Constants::DUMP_INDENT) + "\n";
}

std::string BundleManagerShellCommand::DumpShared(const std::string &bundleName) const
{
    APP_LOGD("DumpShared bundleName: %{public}s", bundleName.c_str());
    SharedBundleInfo sharedBundleInfo;
    ErrCode ret = bundleMgrProxy_->GetSharedBundleInfoBySelf(bundleName, sharedBundleInfo);
    nlohmann::json sharedBundleInfoJson;
    if (ret != ERR_OK) {
        APP_LOGE("dump-shared failed due to errcode %{public}d", ret);
        return std::string();
    } else {
        sharedBundleInfoJson = nlohmann::json {{SHARED_BUNDLE_INFO, sharedBundleInfo}};
    }
    return sharedBundleInfoJson.dump(Constants::DUMP_INDENT);
}

std::string BundleManagerShellCommand::DumpSharedAll() const
{
    APP_LOGD("DumpSharedAll");
    std::string dumpResults = "";
    std::vector<SharedBundleInfo> sharedBundleInfos;
    ErrCode ret = bundleMgrProxy_->GetAllSharedBundleInfo(sharedBundleInfos);
    if (ret != ERR_OK) {
        APP_LOGE("dump-shared all failed due to errcode %{public}d", ret);
        return dumpResults;
    }
    for (const auto& item : sharedBundleInfos) {
        dumpResults.append("\t");
        dumpResults.append(item.name);
        dumpResults.append("\n");
    }
    return dumpResults;
}

// Clean helper methods

bool BundleManagerShellCommand::CleanBundleCacheFilesOperation(const std::string &bundleName, int32_t userId,
    int32_t appIndex) const
{
    userId = BundleCommandCommon::GetCurrentUserId(userId);
    APP_LOGD("bundleName: %{public}s, userId:%{public}d, appIndex:%{public}d", bundleName.c_str(), userId, appIndex);
    sptr<CleanCacheCallbackImpl> cleanCacheCallBack(new (std::nothrow) CleanCacheCallbackImpl());
    if (cleanCacheCallBack == nullptr) {
        APP_LOGE("cleanCacheCallBack is null");
        return false;
    }
    ErrCode cleanRet = bundleMgrProxy_->CleanBundleCacheFiles(bundleName, cleanCacheCallBack, userId, appIndex);
    if (cleanRet == ERR_OK) {
        return cleanCacheCallBack->GetResultCode();
    }
    APP_LOGE("clean bundle cache files operation failed, cleanRet = %{public}d", cleanRet);
    return false;
}

bool BundleManagerShellCommand::CleanBundleDataFilesOperation(const std::string &bundleName, int32_t userId,
    int32_t appIndex) const
{
    userId = BundleCommandCommon::GetCurrentUserId(userId);
    APP_LOGD("bundleName: %{public}s, userId:%{public}d, appIndex:%{public}d", bundleName.c_str(), userId, appIndex);
    auto appMgrClient = std::make_unique<AppMgrClient>();
    APP_LOGI("clear start");
    ErrCode cleanRetAms = appMgrClient->ClearUpApplicationData(bundleName, appIndex, userId);
    APP_LOGI("clear end");
    bool cleanRetBms = bundleMgrProxy_->CleanBundleDataFiles(bundleName, userId, appIndex);
    APP_LOGD("cleanRetAms: %{public}d, cleanRetBms: %{public}d", cleanRetAms, cleanRetBms);
    if ((cleanRetAms == ERR_OK) && cleanRetBms) {
        return true;
    }
    APP_LOGE("clean bundle data files operation failed");
    return false;
}

// Common helper methods

void BundleManagerShellCommand::GetAbsPaths(const std::vector<std::string> &paths,
    std::vector<std::string> &absPaths) const
{
    std::vector<std::string> realPathVec;
    for (const auto &bundlePath : paths) {
        std::string absoluteBundlePath;
        if (bundlePath[0] == '/') {
            // absolute path
            absoluteBundlePath.append(bundlePath);
        } else {
            // relative path
            char *currentPathPtr = getcwd(nullptr, 0);

            if (currentPathPtr != nullptr) {
                absoluteBundlePath.append(currentPathPtr);
                absoluteBundlePath.append('/' + bundlePath);

                free(currentPathPtr);
                currentPathPtr = nullptr;
            }
        }
        realPathVec.emplace_back(absoluteBundlePath);
    }

    for (const auto &path : realPathVec) {
        if (std::find(absPaths.begin(), absPaths.end(), path) == absPaths.end()) {
            absPaths.emplace_back(path);
        }
    }
}

int32_t BundleManagerShellCommand::InstallOperation(const std::vector<std::string> &bundlePaths,
    InstallParam &installParam, int32_t waittingTime, std::string &resultMsg) const
{
    std::vector<std::string> pathVec;
    GetAbsPaths(bundlePaths, pathVec);

    std::vector<std::string> hspPathVec;
    GetAbsPaths(installParam.sharedBundleDirPaths, hspPathVec);
    installParam.sharedBundleDirPaths = hspPathVec;

    sptr<StatusReceiverImpl> statusReceiver(new (std::nothrow) StatusReceiverImpl(waittingTime));
    if (statusReceiver == nullptr) {
        APP_LOGE("statusReceiver is null");
        return IStatusReceiver::ERR_UNKNOWN;
    }
    sptr<BundleDeathRecipient> recipient(new (std::nothrow) BundleDeathRecipient(statusReceiver));
    if (recipient == nullptr) {
        APP_LOGE("recipient is null");
        return IStatusReceiver::ERR_UNKNOWN;
    }
    bundleInstallerProxy_->AsObject()->AddDeathRecipient(recipient);
    ErrCode res = bundleInstallerProxy_->StreamInstall(pathVec, installParam, statusReceiver);
    APP_LOGD("StreamInstall result is %{public}d", res);
    if (res == ERR_OK) {
        resultMsg = statusReceiver->GetResultMsg();
        return statusReceiver->GetResultCode();
    }
    if (res == ERR_APPEXECFWK_INSTALL_PARAM_ERROR) {
        APP_LOGE("install param error");
        return IStatusReceiver::ERR_INSTALL_PARAM_ERROR;
    }
    if (res == ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR) {
        APP_LOGE("install internal error");
        return IStatusReceiver::ERR_INSTALL_INTERNAL_ERROR;
    }
    if (res == ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID) {
        APP_LOGE("install invalid path");
        return IStatusReceiver::ERR_INSTALL_FILE_PATH_INVALID;
    }
    if (res == ERR_APPEXECFWK_INSTALL_DISK_MEM_INSUFFICIENT) {
        APP_LOGE("install failed due to no space left");
        return IStatusReceiver::ERR_INSTALL_DISK_MEM_INSUFFICIENT;
    }

    return res;
}

int32_t BundleManagerShellCommand::UninstallOperation(
    const std::string &bundleName, const std::string &moduleName, InstallParam &installParam) const
{
    sptr<StatusReceiverImpl> statusReceiver(new (std::nothrow) StatusReceiverImpl());
    if (statusReceiver == nullptr) {
        APP_LOGE("statusReceiver is null");
        return IStatusReceiver::ERR_UNKNOWN;
    }

    APP_LOGD("bundleName: %{public}s", bundleName.c_str());
    APP_LOGD("moduleName: %{public}s", moduleName.c_str());

    sptr<BundleDeathRecipient> recipient(new (std::nothrow) BundleDeathRecipient(statusReceiver));
    if (recipient == nullptr) {
        APP_LOGE("recipient is null");
        return IStatusReceiver::ERR_UNKNOWN;
    }
    bundleInstallerProxy_->AsObject()->AddDeathRecipient(recipient);
    if (moduleName.size() != 0) {
        bundleInstallerProxy_->Uninstall(bundleName, moduleName, installParam, statusReceiver);
    } else {
        bundleInstallerProxy_->Uninstall(bundleName, installParam, statusReceiver);
    }

    return statusReceiver->GetResultCode();
}

int32_t BundleManagerShellCommand::UninstallSharedOperation(const UninstallParam &uninstallParam) const
{
    sptr<StatusReceiverImpl> statusReceiver(new (std::nothrow) StatusReceiverImpl());
    if (statusReceiver == nullptr) {
        APP_LOGE("statusReceiver is null");
        return IStatusReceiver::ERR_UNKNOWN;
    }

    sptr<BundleDeathRecipient> recipient(new (std::nothrow) BundleDeathRecipient(statusReceiver));
    if (recipient == nullptr) {
        APP_LOGE("recipient is null");
        return IStatusReceiver::ERR_UNKNOWN;
    }
    bundleInstallerProxy_->AsObject()->AddDeathRecipient(recipient);

    bundleInstallerProxy_->Uninstall(uninstallParam, statusReceiver);
    return statusReceiver->GetResultCode();
}

std::string BundleManagerShellCommand::GetWaringString(int32_t currentUserId, int32_t specifedUserId) const
{
    std::string warning = WARNING_USER;
    warning.replace(warning.find_first_of('%'), 1, std::to_string(currentUserId));
    warning.replace(warning.find_first_of('$'), 1, std::to_string(specifedUserId));
    warning.replace(warning.find_first_of('$'), 1, std::to_string(specifedUserId));
    return warning;
}

ErrCode BundleManagerShellCommand::RunAsSetDisposedRuleCommand()
{
    APP_LOGI("begin to RunAsSetDisposedRuleCommand");
    if (argc_ <= 2) {
        resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    int32_t result = OHOS::ERR_OK;
    int32_t counter = 0;
    std::string appId;
    int32_t appIndex = 0;
    bool hasAppId = false;
    bool hasPriority = false;
    bool hasComponentType = false;
    bool hasDisposedType = false;
    bool hasControlType = false;
    bool hasWantBundleName = false;
    bool hasWantAbilityName = false;
    DisposedRule disposedRule;
    std::string wantBundleName;
    std::string wantModuleName;
    std::string wantAbilityName;
    std::vector<std::pair<std::string, std::string>> wantParamsString;
    std::vector<std::pair<std::string, int32_t>> wantParamsInt;
    std::vector<std::pair<std::string, bool>> wantParamsBool;

    const std::string setDisposedRuleOptions = "h";
    const struct option setDisposedRuleLongOptions[] = {
        {"help",               no_argument,       nullptr, 'h'},
        {"appId",              required_argument, nullptr, OPTION_APP_ID},
        {"appIndex",           required_argument, nullptr, OPTION_APP_INDEX},
        {"priority",           required_argument, nullptr, OPTION_PRIORITY},
        {"componentType",      required_argument, nullptr, OPTION_COMPONENT_TYPE},
        {"disposedType",       required_argument, nullptr, OPTION_DISPOSED_TYPE},
        {"controlType",        required_argument, nullptr, OPTION_CONTROL_TYPE},
        {"elements",           required_argument, nullptr, OPTION_ELEMENT},
        {"wantBundleName",     required_argument, nullptr, OPTION_WANT_BUNDLE_NAME},
        {"wantModuleName",     required_argument, nullptr, OPTION_WANT_MODULE_NAME},
        {"wantAbilityName",    required_argument, nullptr, OPTION_WANT_ABILITY_NAME},
        {"wantParamsStrings",  required_argument, nullptr, OPTION_WANT_PS},
        {"wantParamsInts",     required_argument, nullptr, OPTION_WANT_PI},
        {"wantParamsBools",    required_argument, nullptr, OPTION_WANT_PB},
        {nullptr, 0, nullptr, 0},
    };

    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, setDisposedRuleOptions.c_str(),
            setDisposedRuleLongOptions, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    APP_LOGD("'ohos-bm set-disposed-rule' with no option.");
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", HELP_MSG_NO_OPTION);
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", unknownOptionMsg);
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }

        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm set-disposed-rule %{public}s'", argv_[optind - 1]);
                resultReceiver_ = CreateSuccessResult(HELP_MSG_SET_DISPOSED_RULE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case OPTION_APP_ID: {
                appId = optarg;
                hasAppId = true;
                break;
            }
            case OPTION_APP_INDEX: {
                if (!OHOS::StrToInt(optarg, appIndex) || appIndex < 0) {
                    APP_LOGE("ohos-bm set-disposed-rule with error appIndex %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            case OPTION_PRIORITY: {
                if (!OHOS::StrToInt(optarg, disposedRule.priority)) {
                    APP_LOGE("ohos-bm set-disposed-rule with error priority %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                hasPriority = true;
                break;
            }
            case OPTION_COMPONENT_TYPE: {
                int32_t typeValue = 0;
                if (!OHOS::StrToInt(optarg, typeValue) ||
                    typeValue < static_cast<int32_t>(ComponentType::UI_ABILITY) ||
                    typeValue > static_cast<int32_t>(ComponentType::UI_EXTENSION)) {
                    APP_LOGE("ohos-bm set-disposed-rule with error componentType %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                disposedRule.componentType = static_cast<ComponentType>(typeValue);
                hasComponentType = true;
                break;
            }
            case OPTION_DISPOSED_TYPE: {
                int32_t typeValue = 0;
                if (!OHOS::StrToInt(optarg, typeValue) ||
                    typeValue < static_cast<int32_t>(DisposedType::BLOCK_APPLICATION) ||
                    typeValue > static_cast<int32_t>(DisposedType::NON_BLOCK)) {
                    APP_LOGE("ohos-bm set-disposed-rule with error disposedType %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                disposedRule.disposedType = static_cast<DisposedType>(typeValue);
                hasDisposedType = true;
                break;
            }
            case OPTION_CONTROL_TYPE: {
                int32_t typeValue = 0;
                if (!OHOS::StrToInt(optarg, typeValue) ||
                    typeValue < static_cast<int32_t>(ControlType::ALLOWED_LIST) ||
                    typeValue > static_cast<int32_t>(ControlType::DISALLOWED_LIST)) {
                    APP_LOGE("ohos-bm set-disposed-rule with error controlType %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                disposedRule.controlType = static_cast<ControlType>(typeValue);
                hasControlType = true;
                break;
            }
            case OPTION_ELEMENT: {
                std::string elementUri = optarg;
                // Ensure URI starts with '/' for empty deviceId: /bundleName/moduleName/abilityName
                if (elementUri.empty() || elementUri[0] != '/') {
                    elementUri = "/" + elementUri;
                }
                ElementName elementName;
                if (!elementName.ParseURI(elementUri)) {
                    APP_LOGE("parse elementName failed: %{public}s", elementUri.c_str());
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE",
                        "error: --elements format should be /bundleName/moduleName/abilityName.");
                    return OHOS::ERR_INVALID_VALUE;
                }
                disposedRule.elementList.emplace_back(elementName);
                break;
            }
            case OPTION_WANT_BUNDLE_NAME: {
                wantBundleName = optarg;
                hasWantBundleName = true;
                break;
            }
            case OPTION_WANT_MODULE_NAME: {
                wantModuleName = optarg;
                break;
            }
            case OPTION_WANT_ABILITY_NAME: {
                wantAbilityName = optarg;
                hasWantAbilityName = true;
                break;
            }
            case OPTION_WANT_PS: {
                if (optind >= argc_ || argv_[optind] == nullptr ||
                    std::string(argv_[optind]).substr(0, 1) == "-") {
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE",
                        "error: --wantParamsStrings requires key and value.");
                    return OHOS::ERR_INVALID_VALUE;
                }
                wantParamsString.emplace_back(std::make_pair(std::string(optarg),
                    std::string(argv_[optind])));
                optind++;
                break;
            }
            case OPTION_WANT_PI: {
                if (optind >= argc_ || argv_[optind] == nullptr ||
                    std::string(argv_[optind]).substr(0, 1) == "-") {
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE",
                        "error: --wantParamsInts requires key and value.");
                    return OHOS::ERR_INVALID_VALUE;
                }
                int32_t intValue = 0;
                if (!OHOS::StrToInt(argv_[optind], intValue)) {
                    APP_LOGE("ohos-bm set-disposed-rule --wantParamsInts with error value");
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                wantParamsInt.emplace_back(std::make_pair(std::string(optarg), intValue));
                optind++;
                break;
            }
            case OPTION_WANT_PB: {
                if (optind >= argc_ || argv_[optind] == nullptr ||
                    std::string(argv_[optind]).substr(0, 1) == "-") {
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE",
                        "error: --wantParamsBools requires key and value.");
                    return OHOS::ERR_INVALID_VALUE;
                }
                std::string boolStr = argv_[optind];
                bool boolValue = (boolStr == "true" || boolStr == "1");
                wantParamsBool.emplace_back(std::make_pair(std::string(optarg), boolValue));
                optind++;
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    // Check required parameters
    if (result == OHOS::ERR_OK && resultReceiver_ == "") {
        if (!hasAppId || !hasPriority || !hasComponentType ||
            !hasDisposedType || !hasControlType ||
            !hasWantBundleName || !hasWantAbilityName) {
            APP_LOGD("'ohos-bm set-disposed-rule' missing required options.");
            resultReceiver_ = CreateErrorResult("ERR_MISSING_PARAM",
                "error: --appId, --priority, --componentType, "
                "--disposedType, --controlType, --wantBundleName, "
                "--wantAbilityName are required.");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", HELP_MSG_SET_DISPOSED_RULE);
        return result;
    }

    // Get userId from uid
    int32_t userId = BundleCommandCommon::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingTokenID());
    if (userId == Constants::DEFAULT_USERID) {
        APP_LOGE("userId is 0, forbidden to call set-disposed-rule");
        resultReceiver_ = CreateErrorResult("ERR_INVALID_USERID", STRING_USER_ID_INVALID);
        return OHOS::ERR_INVALID_VALUE;
    }

    // Build Want (want-bundle-name, want-module-name, want-ability-name are required)
    auto want = std::make_shared<AAFwk::Want>();
    ElementName elementName("", wantBundleName, wantAbilityName, wantModuleName);
    want->SetElement(elementName);
    for (const auto &param : wantParamsString) {
        want->SetParam(param.first, param.second);
    }
    for (const auto &param : wantParamsInt) {
        want->SetParam(param.first, param.second);
    }
    for (const auto &param : wantParamsBool) {
        want->SetParam(param.first, param.second);
    }
    disposedRule.want = want;

    // Call IPC interface
    if (InitAppControlProxy() != OHOS::ERR_OK && appControlProxy_ == nullptr) {
        APP_LOGE("appControlProxy_ is null");
        resultReceiver_ = CreateErrorResult("ERR_SERVICE_UNAVAILABLE", "error: failed to get app control proxy.");
        return OHOS::ERR_INVALID_VALUE;
    }

    ErrCode res = appControlProxy_->SetDisposedRuleForCloneApp(appId, disposedRule, appIndex, userId);
    if (res == OHOS::ERR_OK) {
        resultReceiver_ = CreateSuccessResult(STRING_SET_DISPOSED_RULE_OK);
    } else {
        resultReceiver_ = CreateErrorResult(static_cast<int32_t>(res), STRING_SET_DISPOSED_RULE_NG);
    }

    APP_LOGI("end");
    return res;
}

ErrCode BundleManagerShellCommand::RunAsDeleteDisposedRuleCommand()
{
    APP_LOGI("begin to RunAsDeleteDisposedRuleCommand");
    if (argc_ <= 2) {
        resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    int32_t result = OHOS::ERR_OK;
    int32_t counter = 0;
    std::string appId;
    int32_t appIndex = 0;
    bool hasAppId = false;

    const std::string deleteDisposedRuleOptions = "h";
    const struct option deleteDisposedRuleLongOptions[] = {
        {"help",      no_argument,       nullptr, 'h'},
        {"appId",     required_argument, nullptr, OPTION_APP_ID},
        {"appIndex",  required_argument, nullptr, OPTION_APP_INDEX},
        {nullptr, 0, nullptr, 0},
    };

    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, deleteDisposedRuleOptions.c_str(),
            deleteDisposedRuleLongOptions, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    APP_LOGD("'ohos-bm delete-disposed-rule' with no option.");
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", HELP_MSG_NO_OPTION);
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", unknownOptionMsg);
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }

        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm delete-disposed-rule %{public}s'", argv_[optind - 1]);
                resultReceiver_ = CreateSuccessResult(HELP_MSG_DELETE_DISPOSED_RULE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case OPTION_APP_ID: {
                appId = optarg;
                hasAppId = true;
                break;
            }
            case OPTION_APP_INDEX: {
                if (!OHOS::StrToInt(optarg, appIndex) || appIndex < 0) {
                    APP_LOGE("ohos-bm delete-disposed-rule with error appIndex %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    // Check required parameters
    if (result == OHOS::ERR_OK && resultReceiver_ == "") {
        if (!hasAppId) {
            APP_LOGD("'ohos-bm delete-disposed-rule' missing required options.");
            resultReceiver_ = CreateErrorResult("ERR_MISSING_PARAM",
                "error: --appId is required.");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_ = CreateErrorResult("ERR_INVALID_VALUE", HELP_MSG_DELETE_DISPOSED_RULE);
        return result;
    }

    // Get userId from uid
    int32_t userId = BundleCommandCommon::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingTokenID());
    if (userId == Constants::DEFAULT_USERID) {
        APP_LOGE("userId is 0, forbidden to call delete-disposed-rule");
        resultReceiver_ = CreateErrorResult("ERR_INVALID_USERID", STRING_USER_ID_INVALID);
        return OHOS::ERR_INVALID_VALUE;
    }

    // Call IPC interface
    if (InitAppControlProxy() != OHOS::ERR_OK && appControlProxy_ == nullptr) {
        APP_LOGE("appControlProxy_ is null");
        resultReceiver_ = CreateErrorResult("ERR_SERVICE_UNAVAILABLE", "error: failed to get app control proxy.");
        return OHOS::ERR_INVALID_VALUE;
    }

    ErrCode res = appControlProxy_->DeleteDisposedRuleForCloneApp(appId, appIndex, userId);
    if (res == OHOS::ERR_OK) {
        resultReceiver_ = CreateSuccessResult(STRING_DELETE_DISPOSED_RULE_OK);
    } else {
        resultReceiver_ = CreateErrorResult(static_cast<int32_t>(res), STRING_DELETE_DISPOSED_RULE_NG);
    }

    APP_LOGI("end");
    return res;
}

}  // namespace AppExecFwk
}  // namespace OHOS
