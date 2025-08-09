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
#include "bundle_command.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <future>
#include <getopt.h>
#include <unistd.h>
#include <vector>
#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "bundle_command_common.h"
#include "bundle_death_recipient.h"
#include "bundle_mgr_client.h"
#include "bundle_mgr_proxy.h"
#include "clean_cache_callback_host.h"
#include "json_serializer.h"
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "parameters.h"
#include "quick_fix_command.h"
#include "quick_fix_status_callback_host_impl.h"
#include "status_receiver_impl.h"
#include "string_ex.h"
#include "app_mgr_client.h"
#include "directory_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BUNDLE_NAME_EMPTY = "";
const std::string OVERLAY_MODULE_INFOS = "overlayModuleInfos";
const std::string OVERLAY_BUNDLE_INFOS = "overlayBundleInfos";
const std::string OVERLAY_MODULE_INFO = "overlayModuleInfo";
const std::string SHARED_BUNDLE_INFO = "sharedBundleInfo";
const std::string DEPENDENCIES = "dependencies";
const char* IS_ROOT_MODE_PARAM = "const.debuggable";
const std::string IS_DEVELOPER_MODE_PARAM = "const.security.developermode.state";
const int32_t ROOT_MODE = 1;
const int32_t USER_MODE = 0;
const int32_t INDEX_OFFSET = 2;
const int32_t MAX_WAITING_TIME = 3000;
const int32_t DEVICE_UDID_LENGTH = 65;
const int32_t MAX_ARGUEMENTS_NUMBER = 3;
const int32_t MAX_OVERLAY_ARGUEMENTS_NUMBER = 8;
const int32_t MINIMUM_WAITTING_TIME = 180; // 3 mins
const int32_t MAXIMUM_WAITTING_TIME = 600; // 10 mins
const int32_t INITIAL_SANDBOX_APP_INDEX = 1000;

const std::string SHORT_OPTIONS_COMPILE = "hm:r:";
const struct option LONG_OPTIONS_COMPILE[] = {
    {"help", no_argument, nullptr, 'h'},
    {"mode", required_argument, nullptr, 'm'},
    {"reset", required_argument, nullptr, 'r'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_COPY_AP = "hn:a";
const struct option LONG_OPTIONS_COPY_AP[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"all", no_argument, nullptr, 'a'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_INSTALL_PLUGIN = "hn:p:";
const struct option LONG_OPTIONS_INSTALL_PLUGIN[] = {
    {"help", no_argument, nullptr, 'h'},
    {"host-bundle-name", required_argument, nullptr, 'n'},
    {"plugin-path", required_argument, nullptr, 'p'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_UNINSTALL_PLUGIN = "hn:p:";
const struct option LONG_OPTIONS_UNINSTALL_PLUGIN[] = {
    {"help", no_argument, nullptr, 'h'},
    {"host-bundle-name", required_argument, nullptr, 'n'},
    {"plugin-bundle-name", required_argument, nullptr, 'p'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS = "hp:rn:m:a:cdu:w:s:i:";
const struct option LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-path", required_argument, nullptr, 'p'},
    {"replace", no_argument, nullptr, 'r'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"module-name", required_argument, nullptr, 'm'},
    {"ability-name", required_argument, nullptr, 'a'},
    {"bundle-info", no_argument, nullptr, 'i'},
    {"cache", no_argument, nullptr, 'c'},
    {"data", no_argument, nullptr, 'd'},
    {"is-removable", required_argument, nullptr, 'i'},
    {"user-id", required_argument, nullptr, 'u'},
    {"waitting-time", required_argument, nullptr, 'w'},
    {"keep-data", no_argument, nullptr, 'k'},
    {"shared-bundle-dir-path", required_argument, nullptr, 's'},
    {"app-index", required_argument, nullptr, 'i'},
    {nullptr, 0, nullptr, 0},
};

const std::string UNINSTALL_OPTIONS = "hn:km:u:v:s";
const struct option UNINSTALL_LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"module-name", required_argument, nullptr, 'm'},
    {"user-id", required_argument, nullptr, 'u'},
    {"keep-data", no_argument, nullptr, 'k'},
    {"version", required_argument, nullptr, 'v'},
    {"shared", no_argument, nullptr, 's'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_DUMP = "hn:aisu:d:gl";
const struct option LONG_OPTIONS_DUMP[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"all", no_argument, nullptr, 'a'},
    {"bundle-info", no_argument, nullptr, 'i'},
    {"shortcut-info", no_argument, nullptr, 's'},
    {"user-id", required_argument, nullptr, 'u'},
    {"device-id", required_argument, nullptr, 'd'},
    {"debug-bundle", no_argument, nullptr, 'g'},
    {"label", no_argument, nullptr, 'l'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_GET = "hu";
const struct option LONG_OPTIONS_GET[] = {
    {"help", no_argument, nullptr, 'h'},
    {"udid", no_argument, nullptr, 'u'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_OVERLAY = "hb:m:t:u:";
const struct option LONG_OPTIONS_OVERLAY[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-name", required_argument, nullptr, 'b'},
    {"module-name", required_argument, nullptr, 'm'},
    {"target-module-name", required_argument, nullptr, 't'},
    {"user-id", required_argument, nullptr, 'u'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_OVERLAY_TARGET = "hb:m:u:";
const struct option LONG_OPTIONS_OVERLAY_TARGET[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-name", required_argument, nullptr, 'b'},
    {"module-name", required_argument, nullptr, 'm'},
    {"user-id", required_argument, nullptr, 'u'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_DUMP_SHARED_DEPENDENCIES = "hn:m:";
const struct option LONG_OPTIONS_DUMP_SHARED_DEPENDENCIES[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"module-name", required_argument, nullptr, 'm'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_DUMP_SHARED = "hn:a";
const struct option LONG_OPTIONS_DUMP_SHARED[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle-name", required_argument, nullptr, 'n'},
    {"all", no_argument, nullptr, 'a'},
    {nullptr, 0, nullptr, 0},
};
}  // namespace

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

std::map<int32_t, int32_t> BundleManagerShellCommand::errCodeMap_ = {
    {ERR_APPEXECFWK_PLUGIN_INSTALL_CHECK_PLUGINID_ERROR, IStatusReceiver::ERR_PLUGIN_INSTALL_CHECK_PLUGINID_ERROR},
    {ERR_APPEXECFWK_SUPPORT_PLUGIN_PERMISSION_ERROR, IStatusReceiver::ERR_PLUGIN_SUPPORT_PLUGIN_PERMISSION_ERROR},
    {ERR_APPEXECFWK_HOST_APPLICATION_NOT_FOUND, IStatusReceiver::ERR_HOST_APP_NOT_FOUND},
    {ERR_APPEXECFWK_INSTALL_VERSION_DOWNGRADE, IStatusReceiver::ERR_INSTALL_VERSION_DOWNGRADE},
    {ERR_APPEXECFWK_INSTALL_FAILED_INCONSISTENT_SIGNATURE, IStatusReceiver::ERR_INSTALL_FAILED_INCONSISTENT_SIGNATURE},
    {ERR_APPEXECFWK_PLUGIN_PARSER_ERROR, IStatusReceiver::ERR_INSTALL_PARSE_FAILED},
    {ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID, IStatusReceiver::ERR_INSTALL_FILE_PATH_INVALID},
    {ERR_APPEXECFWK_INSTALL_INVALID_BUNDLE_FILE, IStatusReceiver::ERR_INSTALL_INVALID_BUNDLE_FILE},
    {ERR_APPEXECFWK_INSTALL_INVALID_HAP_SIZE, IStatusReceiver::ERR_INSTALL_INVALID_HAP_SIZE},
    {ERR_APPEXECFWK_INSTALL_FAILED_INCOMPATIBLE_SIGNATURE, IStatusReceiver::ERR_INSTALL_FAILED_INCOMPATIBLE_SIGNATURE},
    {ERR_APPEXECFWK_INSTALL_FAILED_BUNDLE_SIGNATURE_VERIFICATION_FAILURE,
        IStatusReceiver::ERR_INSTALL_FAILED_BUNDLE_SIGNATURE_VERIFICATION_FAILURE},
    {ERR_APPEXECFWK_INSTALL_PERMISSION_DENIED, IStatusReceiver::ERR_INSTALL_PERMISSION_DENIED},
    {ERR_APPEXECFWK_INSTALL_SYSCAP_FAILED_AND_DEVICE_TYPE_ERROR,
        IStatusReceiver::ERR_INSTALL_CHECK_SYSCAP_FAILED_AND_DEVICE_TYPE_NOT_SUPPORTED},
    {ERR_APPEXECFWK_USER_NOT_EXIST, IStatusReceiver::ERR_USER_NOT_EXIST},
    {ERR_BUNDLEMANAGER_INSTALL_CODE_SIGNATURE_FAILED, IStatusReceiver::ERR_INSTALL_CODE_SIGNATURE_FAILED},
    {ERR_APPEXECFWK_DEVICE_NOT_SUPPORT_PLUGIN, IStatusReceiver::ERR_DEVICE_NOT_SUPPORT_PLUGIN},
    {ERR_APPEXECFWK_PLUGIN_CHECK_APP_LABEL_ERROR, IStatusReceiver::ERR_MULTIPLE_HSP_INFO_INCONSISTENT},
    {ERR_APPEXECFWK_INSTALL_DEBUG_BUNDLE_NOT_ALLOWED, IStatusReceiver::ERR_INSTALL_DEBUG_BUNDLE_NOT_ALLOWED},
    {ERR_APPEXECFWK_PLUGIN_INSTALL_PARSE_PLUGINID_ERROR, IStatusReceiver::ERR_PLUGIN_ID_PARSE_FAILED},
    {ERR_APPEXECFWK_PLUGIN_NOT_FOUND, IStatusReceiver::ERR_PLUGIN_APP_NOT_FOUND},
    {ERR_APPEXECFWK_PLUGIN_INSTALL_NOT_ALLOW, IStatusReceiver::ERR_INSTALL_PARSE_FAILED},
    {ERR_APPEXECFWK_PLUGIN_INSTALL_SAME_BUNDLE_NAME, IStatusReceiver::ERR_PLUGIN_SAME_BUNDLE_NAME},
    {ERR_APPEXECFWK_INSTALL_PERMISSION_DENIED, IStatusReceiver::ERR_INSTALL_PERMISSION_DENIED},
    {ERR_APPEXECFWK_UNINSTALL_PERMISSION_DENIED, IStatusReceiver::ERR_UNINSTALL_PERMISSION_DENIED},
    {ERR_APPEXECFWK_INSTALL_INVALID_HAP_NAME, IStatusReceiver::ERR_INSTALL_INVALID_HAP_NAME},
    {ERR_APPEXECFWK_INSTALL_DISK_MEM_INSUFFICIENT, IStatusReceiver::ERR_INSTALL_DISK_MEM_INSUFFICIENT}
};

BundleManagerShellCommand::BundleManagerShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, TOOL_NAME)
{}

ErrCode BundleManagerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", [this] { return this->RunAsHelpCommand(); } },
        {"install", [this] { return this->RunAsInstallCommand(); } },
        {"uninstall", [this] { return this->RunAsUninstallCommand(); } },
        {"install-plugin", [this] { return this->RunAsInstallPluginCommand(); } },
        {"uninstall-plugin", [this] { return this->RunAsUninstallPluginCommand(); } },
        {"dump", [this] { return this->RunAsDumpCommand(); } },
        {"clean", [this] { return this->RunAsCleanCommand(); } },
        {"get", [this] { return this->RunAsGetCommand(); } },
        {"quickfix", [this] { return this->RunAsQuickFixCommand(); } },
        {"compile", [this] { return this->RunAsCompileCommand(); } },
        {"copy-ap", [this] { return this->RunAsCopyApCommand(); } },
        {"dump-overlay", [this] { return this->RunAsDumpOverlay(); } },
        {"dump-target-overlay", [this] { return this->RunAsDumpTargetOverlay(); } },
        {"dump-dependencies", [this] { return this->RunAsDumpSharedDependenciesCommand(); } },
        {"dump-shared", [this] { return this->RunAsDumpSharedCommand(); } },
    };

    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, USER_MODE);
    if (mode == ROOT_MODE) {
        commandMap_.emplace("enable", [this] { return this->RunAsEnableCommand(); });
        commandMap_.emplace("disable", [this] { return this->RunAsDisableCommand(); });
    }

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
        if (bundleMgrProxy_) {
            if (bundleInstallerProxy_ == nullptr) {
                bundleInstallerProxy_ = bundleMgrProxy_->GetBundleInstaller();
            }
        }
    }

    if ((bundleMgrProxy_ == nullptr) || (bundleInstallerProxy_ == nullptr) ||
        (bundleInstallerProxy_->AsObject() == nullptr)) {
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode BundleManagerShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);

    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, USER_MODE);
    APP_LOGI("current mode is: %{public}d", mode);
    if (mode == ROOT_MODE) {
        resultReceiver_.append(ENABLE_DISABLE_HELP_MSG);
    }

    bool isDeveloperMode = system::GetBoolParameter(IS_DEVELOPER_MODE_PARAM, false);
    APP_LOGD("current developer mode is: %{public}d", isDeveloperMode);
    if (mode == ROOT_MODE || isDeveloperMode) {
        resultReceiver_.append(CLEAN_HELP_MSG);
    }

    return OHOS::ERR_OK;
}

bool BundleManagerShellCommand::IsInstallOption(int index) const
{
    if (index >= argc_ || index < INDEX_OFFSET) {
        return false;
    }
    if (argList_[index - INDEX_OFFSET] == "-r" || argList_[index - INDEX_OFFSET] == "--replace" ||
        argList_[index - INDEX_OFFSET] == "-p" || argList_[index - INDEX_OFFSET] == "--bundle-path" ||
        argList_[index - INDEX_OFFSET] == "-u" || argList_[index - INDEX_OFFSET] == "--user-id" ||
        argList_[index - INDEX_OFFSET] == "-w" || argList_[index - INDEX_OFFSET] == "--waitting-time" ||
        argList_[index - INDEX_OFFSET] == "-s" || argList_[index - INDEX_OFFSET] == "--shared-bundle-dir-path") {
        return true;
    }
    return false;
}

ErrCode BundleManagerShellCommand::RunAsCopyApCommand()
{
    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, USER_MODE);
    bool isDeveloperMode = system::GetBoolParameter(IS_DEVELOPER_MODE_PARAM, false);
    if (mode != ROOT_MODE && !isDeveloperMode) {
        APP_LOGI("in user mode but not in developer mode");
        return ERR_OK;
    }
    APP_LOGI("begin to RunAsCopyApCommand");
    int result = OHOS::ERR_OK;
    int counter = 0;
    std::string bundleName = "";
    bool isAllBundle = false;
    int32_t option;
    while ((option = getopt_long(argc_, argv_, SHORT_OPTIONS_COPY_AP.c_str(),
        LONG_OPTIONS_COPY_AP, nullptr)) != -1) {
        counter++;
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        result = ParseCopyApCommand(option, bundleName, isAllBundle);
        if (option == '?') {
            break;
        }
    }

    if ((option == -1) && (counter == 0)) {
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
            // 1.'bm copy-ap' with no option: bm copy-ap
            // 2.'bm copy-ap' with a wrong argument: bm copy-ap -xxx
            APP_LOGD("'bm copy-ap' %{public}s", HELP_MSG_NO_OPTION.c_str());
            resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_COPY_AP);
    } else {
        std::string copyApResult = "";
        copyApResult = CopyAp(bundleName, isAllBundle);
        if (copyApResult.empty() || (copyApResult == "")) {
            copyApResult = "parameters may be wrong\n";
        }
        resultReceiver_.append(copyApResult);
    }
    APP_LOGI("end to RunAsCopyApCommand");
    return result;
}

ErrCode BundleManagerShellCommand::ParseCopyApCommand(int32_t option, std::string &bundleName, bool &isAllBundle)
{
    int32_t result = OHOS::ERR_OK;
    if (option == '?') {
        switch (optopt) {
            case 'n': {
                // 'bm copy-ap -n' with no argument: bm copy-ap -n
                // 'bm copy-ap --bundle-name' with no argument: bm copy-ap --bundle-name
                APP_LOGD("'bm copy-ap -n' with no argument.");
                resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                // 'bm copy-ap' with an unknown option: bm copy-ap -x
                // 'bm copy-ap' with an unknown option: bm copy-ap -xxx
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                APP_LOGE("'bm copy-ap' with an unknown option.");
                resultReceiver_.append(unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    } else {
        switch (option) {
            case 'h': {
                // 'bm copy-ap -h'
                // 'bm copy-ap --help'
                APP_LOGD("'bm copy-ap %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'a': {
                // 'bm copy-ap -a'
                // 'bm copy-ap --all'
                APP_LOGD("'bm copy-ap %{public}s'", argv_[optind - 1]);
                isAllBundle = true;
                break;
            }
            case 'n': {
                // 'bm copy-ap -n xxx'
                // 'bm copy-ap --bundle-name xxx'
                APP_LOGD("'bm copy-ap %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                bundleName = optarg;
                break;
            }
            default: {
                APP_LOGD("'bm copy-ap %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    return result;
}

ErrCode BundleManagerShellCommand::RunAsCompileCommand()
{
    APP_LOGI("begin");
    int result = OHOS::ERR_OK;
    int counter = 0;
    std::string compileMode = "";
    std::string bundleName = "";
    bool bundleCompile = false;
    bool resetCompile = false;
    bool isAllBundle = false;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS_COMPILE.c_str(), LONG_OPTIONS_COMPILE, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm compile' with no option: bm compile
                    // 'bm compile' with a wrong argument: bm compile xxx
                    APP_LOGD("'bm compile' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        if (option == '?') {
            switch (optopt) {
                case 'a': {
                    // 'bm compile -m' with no argument: bm compile -m
                    // 'bm compile --mode' with no argument: bm compile --mode
                    APP_LOGD("'bm compile %{public}s'", argv_[optind - 1]);
                    isAllBundle = true;
                    break;
                }
                default: {
                    // 'bm compile' with an unknown option: bm compile -x
                    // 'bm compile' with an unknown option: bm compile -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGE("'bm compile' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                // 'bm compile -h'
                // 'bm compile --help'
                APP_LOGD("'bm compile %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'm': {
                // 'bm compile -m xxx'
                // 'bm compile --mode xxx'
                if (optind + 1 > argc_) {
                    APP_LOGE("out of index");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                if (argv_[optind + 1] == nullptr) {
                    APP_LOGE("'bm compile' with necessarily parameter missing.");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                APP_LOGD("'bm compile %{public}s %{public}s %{public}s'",
                    argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg, argv_[optind + 1]);
                bundleCompile = true;
                compileMode = optarg;
                bundleName = argv_[optind + 1];
                break;
            }
            case 'r': {
                // 'bm compile -r xxx'
                // 'bm compile --reset xxx'
                APP_LOGD("'bm compile %{public}s'", argv_[optind - 1]);
                resetCompile = true;
                bundleName = optarg;
                if (bundleName == "-a") {
                    isAllBundle = true;
                }
                break;
            }
            default: {
                APP_LOGD("'bm compile %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_COMPILE);
    } else {
        std::string compileResults = "";
        APP_LOGD("compileResults: %{public}s", compileResults.c_str());
        if (bundleCompile) {
            compileResults = CompileProcessAot(bundleName, compileMode, isAllBundle);
        } else if (resetCompile) {
            compileResults = CompileReset(bundleName, isAllBundle);
        }
        if (compileResults.empty() || (compileResults == "")) {
            compileResults = HELP_MSG_COMPILE_FAILED + "\n";
        }
        resultReceiver_.append(compileResults);
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsInstallCommand()
{
    APP_LOGI("begin to RunAsInstallCommand");
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
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm install' with no option: bm install
                    // 'bm install' with a wrong argument: bm install xxx
                    APP_LOGD("'bm install' with no option.");
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'p': {
                    // 'bm install -p' with no argument: bm install -p
                    // 'bm install --bundle-path' with no argument: bm install --bundle-path
                    APP_LOGD("'bm install' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm install -u' with no argument: bm install -u
                    // 'bm install --user-id' with no argument: bm install --user-id
                    APP_LOGD("'bm install -u' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'w': {
                    // 'bm install -w' with no argument: bm install -w
                    // 'bm install --waitting-time' with no argument: bm install --waitting-time
                    APP_LOGD("'bm install -w' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm install' with an unknown option: bm install -x
                    // 'bm install' with an unknown option: bm install -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm install' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm install -h'
                // 'bm install --help'
                APP_LOGD("'bm install %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'p': {
                // 'bm install -p <bundle-file-path>'
                // 'bm install --bundle-path <bundle-file-path>'
                APP_LOGD("'bm install %{public}s'", argv_[optind - 1]);
                if (GetBundlePath(optarg, bundlePath) != OHOS::ERR_OK) {
                    APP_LOGD("'bm install' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                index = optind;
                break;
            }
            case 'r': {
                // 'bm install -r'
                // 'bm install --replace'
                installFlag = InstallFlag::REPLACE_EXISTING;
                break;
            }
            case 'u': {
                // 'bm install -p <bundle-file-path> -u userId'
                // 'bm install --bundle-path <bundle-file-path> --user-id userId'
                APP_LOGW("'bm install -u only support user 0'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("bm install with error userId %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != Constants::DEFAULT_USERID && userId != currentUser) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
                }
                break;
            }
            case 'w': {
                APP_LOGD("'bm install %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                if (!OHOS::StrToInt(optarg, waittingTime) || waittingTime < MINIMUM_WAITTING_TIME ||
                    waittingTime > MAXIMUM_WAITTING_TIME) {
                    APP_LOGE("bm install with error waittingTime %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            case 's': {
                // 'bm install -s <hsp-dir-path>'
                // 'bm install --shared-bundle-dir-path <hsp-dir-path>'
                APP_LOGD("'bm install %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                if (GetBundlePath(optarg, sharedBundleDirPaths) != OHOS::ERR_OK) {
                    APP_LOGD("'bm install -s' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                hspIndex = optind;
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
            APP_LOGD("'bm install' with error arguments.");
            resultReceiver_.append("error value for the chosen option");
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
            APP_LOGD("'bm install -s' with error arguments.");
            resultReceiver_.append("error value for the chosen option");
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
            // 'bm install ...' with no bundle path option
            APP_LOGD("'bm install' with no bundle path option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_PATH_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_INSTALL);
    } else {
        InstallParam installParam;
        installParam.installFlag = installFlag;
        installParam.userId = userId;
        installParam.sharedBundleDirPaths = sharedBundleDirPaths;
        std::string resultMsg;
        int32_t installResult = InstallOperation(bundlePath, installParam, waittingTime, resultMsg);
        if (installResult == OHOS::ERR_OK) {
            resultReceiver_ = STRING_INSTALL_BUNDLE_OK + "\n";
        } else {
            resultReceiver_ = STRING_INSTALL_BUNDLE_NG + "\n";
            resultReceiver_.append(GetMessageFromCode(installResult));
            if (!resultMsg.empty() && resultMsg[0] != '[') {
                resultReceiver_.append(resultMsg + "\n");
            }
        }
        if (!warning.empty()) {
            resultReceiver_ = warning + resultReceiver_;
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
        param == "--bundle-path" || param == "-u" || param == "--user-id" ||
        param == "-w" || param == "--waitting-time") {
        return OHOS::ERR_INVALID_VALUE;
    }
    bundlePaths.emplace_back(param);
    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::GetPluginPath(const std::string& param,
    std::vector<std::string>& pluginPaths) const
{
    if (param.empty()) {
        return OHOS::ERR_INVALID_VALUE;
    }
    if (param == "-p" || param == "--plugin-path" ||
        param == "-n" || param == "--host-bundle-name") {
        return OHOS::ERR_INVALID_VALUE;
    }
    pluginPaths.emplace_back(param);
    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::RunAsUninstallCommand()
{
    APP_LOGI("begin to RunAsUninstallCommand");
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
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm uninstall' with no option: bm uninstall
                    // 'bm uninstall' with a wrong argument: bm uninstall xxx
                    APP_LOGD("'bm uninstall' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm uninstall -n' with no argument: bm uninstall -n
                    // 'bm uninstall --bundle-name' with no argument: bm uninstall --bundle-name
                    APP_LOGD("'bm uninstall -n' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'm': {
                    // 'bm uninstall -m' with no argument: bm uninstall -m
                    // 'bm uninstall --module-name' with no argument: bm uninstall --module-name
                    APP_LOGD("'bm uninstall -m' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm uninstall -n <bundleName> -u userId'
                    // 'bm uninstall --bundle-name <bundleName> --user-id userId'
                    APP_LOGD("'bm uninstall -u' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'k': {
                    // 'bm uninstall -n <bundleName> -k'
                    // 'bm uninstall --bundle-name <bundleName> --keep-data'
                    APP_LOGD("'bm uninstall -k'");
                    isKeepData = true;
                    break;
                }
                case 's': {
                    APP_LOGD("'bm uninstall -s'");
                    isShared = true;
                    break;
                }
                case 'v': {
                    APP_LOGD("'bm uninstall -v'");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm uninstall' with an unknown option: bm uninstall -x
                    // 'bm uninstall' with an unknown option: bm uninstall -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm uninstall' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm uninstall -h'
                // 'bm uninstall --help'
                APP_LOGD("'bm uninstall %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm uninstall -n xxx'
                // 'bm uninstall --bundle-name xxx'
                APP_LOGD("'bm uninstall %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                bundleName = optarg;
                break;
            }
            case 'm': {
                // 'bm uninstall -m xxx'
                // 'bm uninstall --module-name xxx'
                APP_LOGD("'bm uninstall %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                moduleName = optarg;
                break;
            }
            case 'u': {
                // 'bm uninstall -n <bundleName> -u userId'
                // 'bm uninstall --bundle-name <bundleName> --user-id userId'
                APP_LOGW("'bm uninstall -u only support user 0'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("bm uninstall with error userId %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != Constants::DEFAULT_USERID && userId != currentUser) {
                    warning = GetWaringString(currentUser, userId);
                    userId = currentUser;
                }
                break;
            }
            case 'k': {
                // 'bm uninstall -n <bundleName> -k'
                // 'bm uninstall --bundle-name <bundleName> --keep-data'
                APP_LOGD("'bm uninstall %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT]);
                isKeepData = true;
                break;
            }
            case 's': {
                APP_LOGD("'bm uninstall -s'");
                isShared = true;
                break;
            }
            case 'v': {
                APP_LOGD("'bm uninstall %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                if (!OHOS::StrToInt(optarg, versionCode) || versionCode < 0) {
                    APP_LOGE("bm uninstall with error versionCode %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
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
            // 'bm uninstall ...' with no bundle name option
            APP_LOGD("'bm uninstall' with bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_UNINSTALL);
        return result;
    }

    if (isShared) {
        UninstallParam uninstallParam;
        uninstallParam.bundleName = bundleName;
        uninstallParam.versionCode = versionCode;
        APP_LOGE("version code is %{public}d", versionCode);
        int32_t uninstallResult = UninstallSharedOperation(uninstallParam);
        if (uninstallResult == OHOS::ERR_OK) {
            resultReceiver_ = STRING_UNINSTALL_BUNDLE_OK + "\n";
        } else {
            resultReceiver_ = STRING_UNINSTALL_BUNDLE_NG + "\n";
            resultReceiver_.append(GetMessageFromCode(uninstallResult));
        }
    } else {
        InstallParam installParam;
        installParam.userId = userId;
        installParam.isKeepData = isKeepData;
        installParam.parameters.emplace(Constants::VERIFY_UNINSTALL_RULE_KEY, Constants::VERIFY_UNINSTALL_RULE_VALUE);
        int32_t uninstallResult = UninstallOperation(bundleName, moduleName, installParam);
        if (uninstallResult == OHOS::ERR_OK) {
            resultReceiver_ = STRING_UNINSTALL_BUNDLE_OK + "\n";
        } else {
            resultReceiver_ = STRING_UNINSTALL_BUNDLE_NG + "\n";
            resultReceiver_.append(GetMessageFromCode(uninstallResult));
        }
        if (!warning.empty()) {
            resultReceiver_ = warning + resultReceiver_;
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsDumpCommand()
{
    APP_LOGI("begin to RunAsDumpCommand");
    int result = OHOS::ERR_OK;
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
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm dump' with no option: bm dump
                    // 'bm dump' with a wrong argument: bm dump xxx
                    APP_LOGD("'bm dump' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm dump -n' with no argument: bm dump -n
                    // 'bm dump --bundle-name' with no argument: bm dump --bundle-name
                    APP_LOGD("'bm dump -n' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm dump -u' with no argument: bm dump -u
                    // 'bm dump --user-id' with no argument: bm dump --user-id
                    APP_LOGD("'bm dump -u' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'd': {
                    // 'bm dump -d' with no argument: bm dump -d
                    // 'bm dump --device-id' with no argument: bm dump --device-id
                    APP_LOGD("'bm dump -d' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm dump' with an unknown option: bm dump -x
                    // 'bm dump' with an unknown option: bm dump -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm dump' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                // 'bm dump -h'
                // 'bm dump --help'
                APP_LOGD("'bm dump %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'a': {
                // 'bm dump -a'
                // 'bm dump --all'
                APP_LOGD("'bm dump %{public}s'", argv_[optind - 1]);
                bundleDumpAll = true;
                break;
            }
            case 'l': {
                // 'bm dump -l'
                // 'bm dump --label'
                APP_LOGD("'bm dump %{public}s'", argv_[optind - 1]);
                bundleDumpLabel = true;
                break;
            }
            case 'g': {
                // 'bm dump -g'
                // 'bm dump --debug-bundle'
                APP_LOGD("'bm dump %{public}s'", argv_[optind - 1]);
                bundleDumpDebug = true;
                break;
            }
            case 'n': {
                // 'bm dump -n xxx'
                // 'bm dump --bundle-name xxx'
                APP_LOGD("'bm dump %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                bundleName = optarg;
                bundleDumpInfo = true;
                break;
            }
            case 's': {
                // 'bm dump -n xxx -s'
                // 'bm dump --bundle-name xxx --shortcut-info'
                APP_LOGD("'bm dump %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                bundleDumpShortcut = true;
                break;
            }
            case 'u': {
                // 'bm dump -n <bundleName> -u userId'
                // 'bm dump --bundle-name <bundleName> --user-id userId'
                APP_LOGW("'bm dump -u is not supported'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("bm dump with error userId %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != Constants::DEFAULT_USERID && userId != currentUser) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
                }
                break;
            }
            case 'd': {
                // 'bm dump -n <bundleName> -d deviceId'
                // 'bm dump --bundle-name <bundleName> --device-id deviceId'
                APP_LOGD("'bm dump %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
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
            // 'bm dump -s ...' with no bundle name option
            APP_LOGD("'bm dump -s' with no bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
        if ((resultReceiver_ == "") && bundleDumpDistributedBundleInfo && (bundleName.size() == 0)) {
            // 'bm dump d ...' with no bundle name option
            APP_LOGD("'bm dump -d' with no bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DUMP);
    } else {
        std::string dumpResults = "";
        APP_LOGD("dumpResults: %{public}s", dumpResults.c_str());
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
        if (dumpResults.empty() || (dumpResults == "")) {
            dumpResults = HELP_MSG_DUMP_FAILED + "\n";
        }
        resultReceiver_.append(dumpResults);
        if (!warning.empty()) {
            resultReceiver_ = warning + resultReceiver_;
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsCleanCommand()
{
    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, USER_MODE);
    bool isDeveloperMode = system::GetBoolParameter(IS_DEVELOPER_MODE_PARAM, false);
    if (mode != ROOT_MODE && !isDeveloperMode) {
        APP_LOGI("in user mode but not in developer mode");
        return ERR_OK;
    }

    APP_LOGI("begin to RunAsCleanCommand");
    int32_t result = OHOS::ERR_OK;
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
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm clean' with no option: bm clean
                    // 'bm clean' with a wrong argument: bm clean xxx
                    APP_LOGD("'bm clean' %{public}s", HELP_MSG_NO_OPTION.c_str());

                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm clean -n' with no argument: bm clean -n
                    // 'bm clean --bundle-name' with no argument: bm clean --bundle-name
                    APP_LOGD("'bm clean -n' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm clean -u' with no argument: bm clean -u
                    // 'bm clean --user-id' with no argument: bm clean --user-id
                    APP_LOGD("'bm clean -u' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'i': {
                    // 'bm clean -i' with no argument: bm clean -i
                    // 'bm clean --app-index' with no argument: bm clean --app-index
                    APP_LOGD("'bm clean -i' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm clean' with an unknown option: bm clear -x
                    // 'bm clean' with an unknown option: bm clear -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm clean' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm clean -h'
                // 'bm clean --help'
                APP_LOGD("'bm clean %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm clean -n xxx'
                // 'bm clean --bundle-name xxx'
                APP_LOGD("'bm clean %{public}s %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT], optarg);
                bundleName = optarg;
                break;
            }
            case 'c': {
                // 'bm clean -c'
                // 'bm clean --cache'
                APP_LOGD("'bm clean %{public}s'", argv_[optind - OFFSET_REQUIRED_ARGUMENT]);
                cleanCache = cleanData ? false : true;
                break;
            }
            case 'd': {
                // 'bm clean -d'
                // 'bm clean --data'
                APP_LOGD("'bm clean %{public}s '", argv_[optind - OFFSET_REQUIRED_ARGUMENT]);
                cleanData = cleanCache ? false : true;
                break;
            }
            case 'u': {
                // 'bm clean -u userId'
                // 'bm clean --user-id userId'
                APP_LOGW("'bm clean -u is not supported'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("bm clean with error userId %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != currentUser) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
                }
                break;
            }
            case 'i': {
                if (!OHOS::StrToInt(optarg, appIndex) || (appIndex < 0 || appIndex > INITIAL_SANDBOX_APP_INDEX)) {
                    APP_LOGE("bm clean with error appIndex %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
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
            // 'bm clean ...' with no bundle name option
            APP_LOGD("'bm clean' with no bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
        if (!cleanCache && !cleanData) {
            APP_LOGD("'bm clean' with no '-c' or '-d' option.");
            resultReceiver_.append(HELP_MSG_NO_DATA_OR_CACHE_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_CLEAN);
    } else {
        // bm clean -c
        if (cleanCache) {
            if (CleanBundleCacheFilesOperation(bundleName, userId, appIndex)) {
                resultReceiver_ = STRING_CLEAN_CACHE_BUNDLE_OK + "\n";
            } else {
                resultReceiver_ = STRING_CLEAN_CACHE_BUNDLE_NG + "\n";
            }
        }
        // bm clean -d
        if (cleanData) {
            if (CleanBundleDataFilesOperation(bundleName, userId, appIndex)) {
                resultReceiver_.append(STRING_CLEAN_DATA_BUNDLE_OK + "\n");
            } else {
                resultReceiver_.append(STRING_CLEAN_DATA_BUNDLE_NG + "\n");
            }
        }
        if (!warning.empty()) {
            resultReceiver_ = warning + resultReceiver_;
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsEnableCommand()
{
    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, USER_MODE);
    if (mode != ROOT_MODE) {
        APP_LOGI("in user mode");
        return ERR_OK;
    }
    APP_LOGI("begin to RunAsEnableCommand");
    int result = OHOS::ERR_OK;
    int counter = 0;
    std::string bundleName = "";
    std::string abilityName = "";
    const int32_t currentUser = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    int32_t userId = currentUser;
    std::string warning;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm enable' with no option: bm enable
                    // 'bm enable' with a wrong argument: bm enable xxx
                    APP_LOGD("'bm enable' with no option.");
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm enable -n' with no argument: bm enable -n
                    // 'bm enable --bundle-name' with no argument: bm enable --bundle-name
                    APP_LOGD("'bm enable -n' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'a': {
                    // 'bm enable -a' with no argument: bm enable -a
                    // 'bm enable --ability-name' with no argument: bm enable --ability-name
                    APP_LOGD("'bm enable -a' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm enable -u' with no argument: bm enable -u
                    // 'bm enable --user-id' with no argument: bm enable --user-id
                    APP_LOGD("'bm enable -u' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm enable' with an unknown option: bm enable -x
                    // 'bm enable' with an unknown option: bm enable -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm enable' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm enable-h'
                // 'bm enable --help'
                APP_LOGD("'bm enable %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm enable -n <bundle-name>'
                // 'bm enable --bundle-name <bundle-name>'
                bundleName = optarg;
                break;
            }
            case 'a': {
                // 'bm enable -a <ability-name>'
                // 'bm enable --ability-name <ability-name>'
                abilityName = optarg;
                break;
            }
            case 'u': {
                // 'bm enable -u userId'
                // 'bm enable --user-id userId'
                APP_LOGW("'bm enable -u is not supported'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("bm enable with error userId %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != currentUser) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
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
            // 'bm enable ...' with no bundle name option
            APP_LOGD("'bm enable' with no bundle name option.");

            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_ENABLE);
    } else {
        AbilityInfo abilityInfo;
        abilityInfo.name = abilityName;
        abilityInfo.bundleName = bundleName;
        bool enableResult = SetApplicationEnabledOperation(abilityInfo, true, userId);
        if (enableResult) {
            resultReceiver_ = STRING_ENABLE_BUNDLE_OK + "\n";
        } else {
            resultReceiver_ = STRING_ENABLE_BUNDLE_NG + "\n";
        }
        if (!warning.empty()) {
            resultReceiver_ = warning + resultReceiver_;
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsDisableCommand()
{
    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, USER_MODE);
    if (mode != ROOT_MODE) {
        APP_LOGI("in user mode");
        return ERR_OK;
    }
    APP_LOGI("begin to RunAsDisableCommand");
    int result = OHOS::ERR_OK;
    int counter = 0;
    std::string bundleName = "";
    std::string abilityName = "";
    const int32_t currentUser = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    int32_t userId = currentUser;
    std::string warning;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm disable' with no option: bm disable
                    // 'bm disable' with a wrong argument: bm disable xxx
                    APP_LOGD("'bm disable' with no option.");
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    // 'bm disable -n' with no argument: bm disable -n
                    // 'bm disable --bundle-name' with no argument: bm disable --bundle-name
                    APP_LOGD("'bm disable' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'a': {
                    // 'bm disable -a' with no argument: bm disable -a
                    // 'bm disable --ability-name' with no argument: bm disable --ability-name
                    APP_LOGD("'bm disable -a' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm disable -u' with no argument: bm disable -u
                    // 'bm disable --user-id' with no argument: bm disable --user-id
                    APP_LOGD("'bm disable -u' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm disable' with an unknown option: bm disable -x
                    // 'bm disable' with an unknown option: bm disable -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm disable' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                // 'bm disable -h'
                // 'bm disable --help'
                APP_LOGD("'bm disable %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm disable -n <bundle-name>'
                // 'bm disable --bundle-name <bundle-name>'
                bundleName = optarg;
                break;
            }
            case 'a': {
                // 'bm disable -a <ability-name>'
                // 'bm disable --ability-name <ability-name>'
                abilityName = optarg;
                break;
            }
            case 'u': {
                // 'bm disable -u userId'
                // 'bm disable --user-id userId'
                APP_LOGW("'bm disable -u is not supported'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("bm disable with error userId %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != currentUser) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
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
            // 'bm disable ...' with no bundle name option
            APP_LOGD("'bm disable' with no bundle name option.");
            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DISABLE);
    } else {
        AbilityInfo abilityInfo;
        abilityInfo.name = abilityName;
        abilityInfo.bundleName = bundleName;
        bool enableResult = SetApplicationEnabledOperation(abilityInfo, false, userId);
        if (enableResult) {
            resultReceiver_ = STRING_DISABLE_BUNDLE_OK + "\n";
        } else {
            resultReceiver_ = STRING_DISABLE_BUNDLE_NG + "\n";
        }
        if (!warning.empty()) {
            resultReceiver_ = warning + resultReceiver_;
        }
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsGetCommand()
{
    APP_LOGI("begin to RunAsGetCommand");
    int result = OHOS::ERR_OK;
    int counter = 0;
    while (true) {
        counter++;
        if (argc_ > MAX_ARGUEMENTS_NUMBER) {
            resultReceiver_.append(HELP_MSG_GET);
            return OHOS::ERR_INVALID_VALUE;
        }
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS_GET.c_str(), LONG_OPTIONS_GET, nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 1.'bm get' with no option: bm get
                    // 2.'bm get' with a wrong argument: bm get -xxx
                    APP_LOGD("'bm get' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'u': {
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                resultReceiver_.append(STRING_INCORRECT_OPTION + "\n");
                break;
            }
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_GET);
        return result;
    }
    resultReceiver_.append(STRING_GET_UDID_OK + "\n");
    resultReceiver_.append(GetUdid() + "\n");
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsQuickFixCommand()
{
    APP_LOGI("begin to RunAsQuickFixCommand");
    for (auto index = INDEX_OFFSET; index < argc_; ++index) {
        APP_LOGD("argv_[%{public}d]: %{public}s", index, argv_[index]);
        std::string opt = argv_[index];
        if ((opt == "-h") || (opt == "--help")) {
            resultReceiver_.append(HELP_MSG_QUICK_FIX);
            APP_LOGI("end");
            return ERR_OK;
        } else if ((opt == "-a") || (opt == "--apply")) {
            if (index >= argc_ - INDEX_OFFSET) {
                resultReceiver_.append("error: option [--apply] is incorrect.\n");
                resultReceiver_.append(HELP_MSG_QUICK_FIX);
                APP_LOGI("end");
                return ERR_INVALID_VALUE;
            }

            std::string argKey = argv_[++index];
            index++;
            if (argKey == "-f" || argKey == "--file-path") {
                std::vector<std::string> quickFixFiles;
                bool isDebug = false;
                std::string targetPath;
                bool isReplace = false;
                // collect value of multi file-path.
                for (; index < argc_ && index >= INDEX_OFFSET; ++index) {
                    if (argList_[index - INDEX_OFFSET] == "-q" || argList_[index - INDEX_OFFSET] == "--query" ||
                        argList_[index - INDEX_OFFSET] == "-b" || argList_[index - INDEX_OFFSET] == "--bundle-name" ||
                        argList_[index - INDEX_OFFSET] == "-a" || argList_[index - INDEX_OFFSET] == "--apply" ||
                        argList_[index - INDEX_OFFSET] == "-f" || argList_[index - INDEX_OFFSET] == "--file-path") {
                        break;
                    } else if (argList_[index - INDEX_OFFSET] == "-d" || argList_[index - INDEX_OFFSET] == "--debug") {
                        isDebug = true;
                        continue;
                    } else if (argList_[index - INDEX_OFFSET] == "-t" || argList_[index - INDEX_OFFSET] == "--target") {
                        if (index + 1 - INDEX_OFFSET >= static_cast<int32_t>(argList_.size())) {
                            continue;
                        }
                        targetPath = argList_[index + 1 - INDEX_OFFSET];
                        index++;
                        continue;
                    } else if (argList_[index - INDEX_OFFSET] == "-o" ||
                        argList_[index - INDEX_OFFSET] == "--overwrite") {
                        isReplace = true;
                        continue;
                    }
                    quickFixFiles.emplace_back(argList_[index - INDEX_OFFSET]);
                }
                APP_LOGI("end");
                if (!targetPath.empty()) {
                    std::shared_ptr<QuickFixResult> deployRes = nullptr;
                    int32_t result = OHOS::ERR_OK;
                    result = DeployQuickFixDisable(quickFixFiles, deployRes, isDebug, targetPath);
                    resultReceiver_.append((result == OHOS::ERR_OK) ? "apply quickfix succeed.\n" :
                        ("apply quickfix failed with errno: " + std::to_string(result) + ".\n"));
                    return result;
                }
                return QuickFixCommand::ApplyQuickFix(quickFixFiles, resultReceiver_, isDebug, isReplace);
            }
        } else if ((opt == "-q") || (opt == "--query")) {
            if (index >= argc_ - INDEX_OFFSET) {
                resultReceiver_.append("error: option [--query] is incorrect.\n");
                resultReceiver_.append(HELP_MSG_QUICK_FIX);
                APP_LOGI("end");
                return ERR_INVALID_VALUE;
            }

            std::string bundleName;
            std::string argKey = argv_[++index];
            std::string argValue = argv_[++index];
            if (argKey == "-b" || argKey == "--bundle-name") {
                bundleName = argValue;
            }
            APP_LOGI("end");
            return QuickFixCommand::GetApplyedQuickFixInfo(bundleName, resultReceiver_);
        } else if ((opt == "-r") || (opt == "--remove")) {
            if (index >= argc_ - INDEX_OFFSET) {
                resultReceiver_.append("error: option [--remove] is incorrect.\n");
                resultReceiver_.append(HELP_MSG_QUICK_FIX);
                APP_LOGI("end");
                return ERR_INVALID_VALUE;
            }

            std::string bundleName;
            std::string argKey = argv_[++index];
            std::string argValue = argv_[++index];
            if (argKey == "-b" || argKey == "--bundle-name") {
                bundleName = argValue;
            }
            APP_LOGI("end");
            std::shared_ptr<QuickFixResult> deleteRes = nullptr;
            int32_t result = OHOS::ERR_OK;
            result = DeleteQuickFix(bundleName, deleteRes);
            resultReceiver_ = (result == OHOS::ERR_OK) ? "delete quick fix successfully\n" :
                "delete quickfix failed with errno: " + std::to_string(result) + ".\n";
            return result;
        } else {
            resultReceiver_.append("error: unknown option.\n");
            resultReceiver_.append(HELP_MSG_QUICK_FIX);
            APP_LOGI("end");
            return ERR_INVALID_VALUE;
        }
    }

    resultReceiver_.append("error: parameter is not enough.\n");
    resultReceiver_.append(HELP_MSG_QUICK_FIX);
    APP_LOGI("end");
    return ERR_INVALID_VALUE;
}

ErrCode BundleManagerShellCommand::RunAsDumpOverlay()
{
    APP_LOGI("begin to RunAsDumpOverlay");
    int result = OHOS::ERR_OK;
    int counter = 0;
    const int32_t currentUser = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    int32_t userId = currentUser;
    std::string warning;
    std::string bundleName = "";
    std::string moduleName = "";
    std::string targetModuleName = "";
    while (true) {
        counter++;
        if (argc_ > MAX_OVERLAY_ARGUEMENTS_NUMBER) {
            resultReceiver_.append(HELP_MSG_OVERLAY);
            return OHOS::ERR_INVALID_VALUE;
        }
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS_OVERLAY.c_str(), LONG_OPTIONS_OVERLAY,
            nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 1.'bm dump-overlay' with no option: bm dump-overlay
                    // 2.'bm dump-overlay' with a wrong argument: bm dump-overlay -xxx
                    APP_LOGD("'bm dump-overlay' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        if (option == '?') {
            switch (optopt) {
                case 'b': {
                    // 'bm dump-overlay -b' with no argument
                    // 'bm dump-overlay --bundle-name' with no argument
                    APP_LOGD("'bm dump-overlay -b' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'm': {
                    // 'bm dump-overlay -m' with no argument: bm enable -m
                    // 'bm dump-overlay --bundle-name' with no argument: bm dump-overlay --bundle-name
                    APP_LOGD("'bm dump-overlay -m' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 't': {
                    // 'bm dump-overlay -t' with no argument
                    // 'bm dump-overlay --target-module-name' with no argument
                    APP_LOGD("'bm dump-overlay -t' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm dump-overlay -u' with no argument: bm dump-overlay -u
                    // 'bm dump-overlay --user-id' with no argument: bm dump-overlay --user-id
                    APP_LOGD("'bm dump-overlay -u' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm dump-overlay' with an unknown option
                    // 'bm dump-overlay' with an unknown option
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm dump-overlay' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm dump-overlay -h'
                // 'bm dump-overlay --help'
                APP_LOGD("'bm dump-overlay %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'b': {
                // 'bm dump-overlay -b <bundle-name>'
                // 'bm dump-overlay --bundle-name <bundle-name>'
                bundleName = optarg;
                break;
            }
            case 'm': {
                // 'bm dump-overlay -m <module-name>'
                // 'bm dump-overlay --module-name <module-name>'
                moduleName = optarg;
                break;
            }
            case 't': {
                // 'bm dump-overlay -t <target-module-name>'
                // 'bm dump-overlay --target-module-name <target-module-name>'
                targetModuleName = optarg;
                break;
            }
            case 'u': {
                APP_LOGW("'bm dump-overlay -u is not supported'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("bm dump-overlay with error userId %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != currentUser) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
                }
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_OVERLAY);
        return result;
    }
#ifdef BUNDLE_FRAMEWORK_OVERLAY_INSTALLATION
    auto res = DumpOverlayInfo(bundleName, moduleName, targetModuleName, userId);
    if (res.empty()) {
        resultReceiver_.append(STRING_DUMP_OVERLAY_NG + "\n");
    } else {
        resultReceiver_.append(STRING_DUMP_OVERLAY_OK + "\n");
        resultReceiver_.append(res + "\n");
    }
#else
    resultReceiver_.append(MSG_ERR_BUNDLEMANAGER_OVERLAY_FEATURE_IS_NOT_SUPPORTED);
#endif
    if (!warning.empty()) {
        resultReceiver_ = warning + resultReceiver_;
    }
    APP_LOGI("end");
    return result;
}

ErrCode BundleManagerShellCommand::RunAsDumpTargetOverlay()
{
    APP_LOGI("begin to RunAsDumpTargetOverlay");
    int result = OHOS::ERR_OK;
    int counter = 0;
    const int32_t currentUser = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    int32_t userId = currentUser;
    std::string warning;
    std::string bundleName = "";
    std::string moduleName = "";
    while (true) {
        counter++;
        if (argc_ > MAX_OVERLAY_ARGUEMENTS_NUMBER) {
            resultReceiver_.append(HELP_MSG_OVERLAY_TARGET);
            return OHOS::ERR_INVALID_VALUE;
        }
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS_OVERLAY_TARGET.c_str(), LONG_OPTIONS_OVERLAY_TARGET,
            nullptr);
        APP_LOGD("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 1.'bm dump-target-overlay' with no option: bm dump-target-overlay
                    // 2.'bm dump-target-overlay' with a wrong argument: bm dump-target-overlay -xxx
                    APP_LOGD("'bm dump-target-overlay' %{public}s", HELP_MSG_NO_OPTION.c_str());
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        if (option == '?') {
            switch (optopt) {
                case 'b': {
                    // 'bm dump-target-overlay -b' with no argument
                    // 'bm dump-target-overlay --bundle-name' with no argument
                    APP_LOGD("'bm dump-target-overlay -b' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'm': {
                    // 'bm dump-target-overlay -m' with no argument: bm enable -m
                    // 'bm dump-target-overlay --bundle-name' with no argument: bm dump-target-overlay --bundle-name
                    APP_LOGD("'bm dump-target-overlay -m' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'u': {
                    // 'bm dump-target-overlay -u' with no argument: bm dump-target-overlay -u
                    // 'bm dump-target-overlay --user-id' with no argument: bm  dump-target-overlay --user-id
                    APP_LOGD("'bm dump-target-overlay -u' with no argument.");
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'bm dump-target-overlay' with an unknown option
                    // 'bm dump-target-overlay' with an unknown option
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'bm dump-target-overlay' with an unknown option.");
                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'bm dump-target-overlay -h'
                // 'bm dump-target-overlay --help'
                APP_LOGD("'bm dump-target-overlay %{public}s'", argv_[optind - 1]);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'b': {
                // 'bm dump-target-overlay -b <bundle-name>'
                // 'bm dump-target-overlay --bundle-name <bundle-name>'
                bundleName = optarg;
                break;
            }
            case 'm': {
                // 'bm dump-target-overlay -m <module-name>'
                // 'bm dump-target-overlay --module-name <module-name>'
                moduleName = optarg;
                break;
            }
            case 'u': {
                APP_LOGW("'bm dump-target-overlay -u is not supported'");
                if (!OHOS::StrToInt(optarg, userId) || userId < 0) {
                    APP_LOGE("bm dump-target-overlay with error userId %{private}s", optarg);
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                if (userId != currentUser) {
                    warning = GetWaringString(currentUser, userId);
                    userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
                }
                break;
            }
            default: {
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_OVERLAY_TARGET);
        return result;
    }
#ifdef BUNDLE_FRAMEWORK_OVERLAY_INSTALLATION
    auto res = DumpTargetOverlayInfo(bundleName, moduleName, userId);
    if (res.empty()) {
        resultReceiver_.append(STRING_DUMP_TARGET_OVERLAY_NG + "\n");
    } else {
        resultReceiver_.append(STRING_DUMP_TARGET_OVERLAY_OK + "\n");
        resultReceiver_.append(res + "\n");
    }
#else
    resultReceiver_.append(MSG_ERR_BUNDLEMANAGER_OVERLAY_FEATURE_IS_NOT_SUPPORTED);
#endif
    if (!warning.empty()) {
        resultReceiver_ = warning + resultReceiver_;
    }
    APP_LOGI("end");
    return result;
}


std::string BundleManagerShellCommand::GetUdid() const
{
    char innerUdid[DEVICE_UDID_LENGTH] = { 0 };
    int ret = GetDevUdid(innerUdid, DEVICE_UDID_LENGTH);
    if (ret != 0) {
        APP_LOGE("GetUdid failed! ret = %{public}d.", ret);
        return STRING_GET_UDID_NG;
    }
    std::string udid = innerUdid;
    return udid;
}

std::string BundleManagerShellCommand::CopyAp(const std::string &bundleName, bool isAllBundle) const
{
    std::string result = "";
    std::vector<std::string> copyApResults;
    ErrCode ret = bundleMgrProxy_->CopyAp(bundleName, isAllBundle, copyApResults);
    if (ret != ERR_OK) {
        APP_LOGE("failed to copy ap! ret = = %{public}d.", ret);
        return "";
    }
    for (const auto &copyApResult : copyApResults) {
        result.append("\t");
        result.append(copyApResult);
        result.append("\n");
    }
    return result;
}

std::string BundleManagerShellCommand::CompileProcessAot(
    const std::string &bundleName, const std::string &compileMode, bool isAllBundle) const
{
    std::vector<std::string> compileResults;
    ErrCode CompileRet = bundleMgrProxy_->CompileProcessAOT(bundleName, compileMode, isAllBundle, compileResults);
    if (CompileRet != ERR_OK) {
        std::string result = "error: compile AOT:\n";
        for (const auto &compileResult : compileResults) {
            result.append("\t");
            result.append(compileResult);
            result.append("\n");
        }
        return result;
    }
    return COMPILE_SUCCESS_OK;
}

std::string BundleManagerShellCommand::CompileReset(const std::string &bundleName, bool isAllBundle) const
{
    std::string ResetResults;
    ErrCode ResetRet = bundleMgrProxy_->CompileReset(bundleName, isAllBundle);
    if (ResetRet == ERR_APPEXECFWK_PARCEL_ERROR) {
        APP_LOGE("failed to reset AOT.");
        return ResetResults;
    }
    ResetResults = COMPILE_RESET;
    return ResetResults;
}

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

std::string BundleManagerShellCommand::DumpDependentModuleNames(
    const std::string &bundleName,
    const std::string &moduleName) const
{
    APP_LOGD("DumpDependentModuleNames bundleName: %{public}s, moduleName: %{public}s",
        bundleName.c_str(), moduleName.c_str());
    std::string dumpResults = "";
    std::vector<std::string> dependentModuleNames;
    bool dumpRet = bundleMgrProxy_->GetAllDependentModuleNames(bundleName, moduleName, dependentModuleNames);
    if (!dumpRet) {
        APP_LOGE("failed to dump dependent module name.");
    } else {
        dumpResults.append("dependent moduleNames:");
        for (const auto &name : dependentModuleNames) {
            dumpResults.append("\n");
            dumpResults.append(name);
        }
        dumpResults.append("\n");
    }
    return dumpResults;
}

void BundleManagerShellCommand::GetAbsPaths(
    const std::vector<std::string> &paths, std::vector<std::string> &absPaths) const
{
    std::vector<std::string> realPathVec;
    for (auto &bundlePath : paths) {
        std::string absoluteBundlePath = "";
        if (bundlePath.empty()) {
            continue;
        }
        if (bundlePath.at(0) == '/') {
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

bool BundleManagerShellCommand::SetApplicationEnabledOperation(const AbilityInfo &abilityInfo,
    bool isEnable, int32_t userId) const
{
    APP_LOGD("bundleName: %{public}s", abilityInfo.bundleName.c_str());
    userId = BundleCommandCommon::GetCurrentUserId(userId);
    int32_t ret;
    if (abilityInfo.name.size() == 0) {
        ret = bundleMgrProxy_->SetApplicationEnabled(abilityInfo.bundleName, isEnable, userId);
    } else {
        ret = bundleMgrProxy_->SetAbilityEnabled(abilityInfo, isEnable, userId);
    }
    if (ret != 0) {
        if (isEnable) {
            APP_LOGE("enable bundle failed");
        } else {
            APP_LOGE("disable bundle failed");
        }
        return false;
    }
    return true;
}

std::string BundleManagerShellCommand::DumpOverlayInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &targetModuleName, int32_t userId)
{
    std::string res = "";
    if ((bundleName.empty()) || (!moduleName.empty() && !targetModuleName.empty())) {
        APP_LOGE("error value of the dump-overlay command options");
        return res;
    }

    auto overlayManagerProxy = bundleMgrProxy_->GetOverlayManagerProxy();
    if (overlayManagerProxy == nullptr) {
        APP_LOGE("overlayManagerProxy is null");
        return res;
    }
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    ErrCode ret = ERR_OK;
    userId = BundleCommandCommon::GetCurrentUserId(userId);
    if (moduleName.empty() && targetModuleName.empty()) {
        ret = overlayManagerProxy->GetAllOverlayModuleInfo(bundleName, overlayModuleInfos, userId);
        if (overlayModuleInfos.empty()) {
            ret = ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_NO_OVERLAY_MODULE_INFO;
        }
    } else if (!moduleName.empty()) {
        ret = overlayManagerProxy->GetOverlayModuleInfo(bundleName, moduleName, overlayModuleInfo, userId);
    } else {
        ret = overlayManagerProxy->GetOverlayModuleInfoForTarget(bundleName, targetModuleName, overlayModuleInfos,
            userId);
        if (overlayModuleInfos.empty()) {
            ret = ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_NO_OVERLAY_MODULE_INFO;
        }
    }
    if (ret != ERR_OK) {
        APP_LOGE("dump-overlay failed due to errcode %{public}d", ret);
        return res;
    }

    nlohmann::json overlayInfoJson;
    if (!overlayModuleInfos.empty()) {
        overlayInfoJson = nlohmann::json {{OVERLAY_MODULE_INFOS, overlayModuleInfos}};
    } else {
        overlayInfoJson = nlohmann::json {{OVERLAY_MODULE_INFO, overlayModuleInfo}};
    }
    return overlayInfoJson.dump(Constants::DUMP_INDENT);
}

std::string BundleManagerShellCommand::DumpTargetOverlayInfo(const std::string &bundleName,
    const std::string &moduleName, int32_t userId)
{
    std::string res = "";
    if (bundleName.empty()) {
        APP_LOGE("error value of the dump-target-overlay command options");
        return res;
    }
    auto overlayManagerProxy = bundleMgrProxy_->GetOverlayManagerProxy();
    if (overlayManagerProxy == nullptr) {
        APP_LOGE("overlayManagerProxy is null");
        return res;
    }

    userId = BundleCommandCommon::GetCurrentUserId(userId);
    ErrCode ret = ERR_OK;
    nlohmann::json overlayInfoJson;
    if (moduleName.empty()) {
        std::vector<OverlayBundleInfo> overlayBundleInfos;
        ret = overlayManagerProxy->GetOverlayBundleInfoForTarget(bundleName, overlayBundleInfos, userId);
        if (ret != ERR_OK || overlayBundleInfos.empty()) {
            APP_LOGE("dump-target-overlay failed due to errcode %{public}d", ret);
            return res;
        }
        overlayInfoJson = nlohmann::json {{OVERLAY_BUNDLE_INFOS, overlayBundleInfos}};
    } else {
        std::vector<OverlayModuleInfo> overlayModuleInfos;
        ret = overlayManagerProxy->GetOverlayModuleInfoForTarget(bundleName, moduleName, overlayModuleInfos, userId);
        if (ret != ERR_OK || overlayModuleInfos.empty()) {
            APP_LOGE("dump-target-overlay failed due to errcode %{public}d", ret);
            return res;
        }
        overlayInfoJson = nlohmann::json {{OVERLAY_MODULE_INFOS, overlayModuleInfos}};
    }
    return overlayInfoJson.dump(Constants::DUMP_INDENT);
}

ErrCode BundleManagerShellCommand::RunAsDumpSharedDependenciesCommand()
{
    APP_LOGI("begin to RunAsDumpSharedDependenciesCommand");
    int32_t result = OHOS::ERR_OK;
    int32_t counter = 0;
    std::string bundleName;
    std::string moduleName;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS_DUMP_SHARED_DEPENDENCIES.c_str(),
            LONG_OPTIONS_DUMP_SHARED_DEPENDENCIES, nullptr);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm dump-dependencies' with no option: bm dump-dependencies
                    // 'bm dump-dependencies' with a wrong argument: bm dump-dependencies xxx
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        result = ParseSharedDependenciesCommand(option, bundleName, moduleName);
        if (option == '?') {
            break;
        }
    }
    if (result == OHOS::ERR_OK) {
        if ((resultReceiver_ == "") && (bundleName.size() == 0 || moduleName.size() == 0)) {
            // 'bm dump-dependencies -n -m ...' with no bundle name option
            resultReceiver_.append(HELP_MSG_NO_REMOVABLE_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DUMP_SHARED_DEPENDENCIES);
    } else {
        std::string dumpResults = DumpSharedDependencies(bundleName, moduleName);
        if (dumpResults.empty() || (dumpResults == "")) {
            dumpResults = HELP_MSG_DUMP_FAILED + "\n";
        }
        resultReceiver_.append(dumpResults);
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
                // 'bm dump-dependencies -n' with no argument: bm dump-dependencies -n
                // 'bm dump-dependencies --bundle-name' with no argument: bm dump-dependencies --bundle-name
                resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'm': {
                // 'bm dump-dependencies -m' with no argument: bm dump-dependencies -m
                // 'bm dump-dependencies --module-name' with no argument: bm dump-dependencies --module-name
                resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                // 'bm dump-dependencies' with an unknown option: bm dump-dependencies -x
                // 'bm dump-dependencies' with an unknown option: bm dump-dependencies -xxx
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_.append(unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    } else {
        switch (option) {
            case 'h': {
                // 'bm dump-dependencies -h'
                // 'bm dump-dependencies --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm dump-dependencies -n xxx'
                // 'bm dump-dependencies --bundle-name xxx'
                bundleName = optarg;
                break;
            }
            case 'm': {
                // 'bm dump-dependencies -m xxx'
                // 'bm dump-dependencies --module-name xxx'
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
    return dependenciesJson.dump(Constants::DUMP_INDENT);
}

ErrCode BundleManagerShellCommand::RunAsDumpSharedCommand()
{
    APP_LOGI("begin to RunAsDumpSharedCommand");
    int32_t result = OHOS::ERR_OK;
    int32_t counter = 0;
    std::string bundleName;
    bool dumpSharedAll = false;
    while (true) {
        counter++;
        int32_t option = getopt_long(argc_, argv_, SHORT_OPTIONS_DUMP_SHARED.c_str(),
            LONG_OPTIONS_DUMP_SHARED, nullptr);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'bm dump-shared' with no option: bm dump-shared
                    // 'bm dump-shared' with a wrong argument: bm dump-shared xxx
                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }
        result = ParseSharedCommand(option, bundleName, dumpSharedAll);
        if (option == '?') {
            break;
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DUMP_SHARED);
    } else if (dumpSharedAll) {
        std::string dumpResults = DumpSharedAll();
        resultReceiver_.append(dumpResults);
    } else {
        if ((resultReceiver_ == "") && (bundleName.size() == 0)) {
            // 'bm dump-shared -n ...' with no bundle name option
            resultReceiver_.append(HELP_MSG_NO_REMOVABLE_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
            return result;
        }
        std::string dumpResults = DumpShared(bundleName);
        if (dumpResults.empty() || (dumpResults == "")) {
            dumpResults = HELP_MSG_DUMP_FAILED + "\n";
        }
        resultReceiver_.append(dumpResults);
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
                // 'bm dump-shared -n' with no argument: bm dump-shared -n
                // 'bm dump-shared --bundle-name' with no argument: bm dump-shared --bundle-name
                resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                // 'bm dump-shared' with an unknown option: bm dump-shared -x
                // 'bm dump-shared' with an unknown option: bm dump-shared -xxx
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_.append(unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    } else {
        switch (option) {
            case 'h': {
                // 'bm dump-shared -h'
                // 'bm dump-shared --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm dump-shared -n xxx'
                // 'bm dump-shared --bundle-name xxx'
                bundleName = optarg;
                break;
            }
            case 'a': {
                // 'bm dump-shared -a'
                // 'bm dump-shared --all'
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

ErrCode BundleManagerShellCommand::DeployQuickFixDisable(const std::vector<std::string> &quickFixFiles,
    std::shared_ptr<QuickFixResult> &quickFixRes, bool isDebug, const std::string &targetPath) const
{
    std::set<std::string> realPathSet;
    for (const auto &quickFixPath : quickFixFiles) {
        std::string realPath;
        if (!PathToRealPath(quickFixPath, realPath)) {
            APP_LOGW("quickFixPath %{public}s is invalid", quickFixPath.c_str());
            continue;
        }
        APP_LOGD("realPath is %{public}s", realPath.c_str());
        realPathSet.insert(realPath);
    }
    std::vector<std::string> pathVec(realPathSet.begin(), realPathSet.end());

    sptr<QuickFixStatusCallbackHostlmpl> callback(new (std::nothrow) QuickFixStatusCallbackHostlmpl());
    if (callback == nullptr || bundleMgrProxy_ == nullptr) {
        APP_LOGE("callback or bundleMgrProxy is null");
        return ERR_BUNDLEMANAGER_QUICK_FIX_INTERNAL_ERROR;
    }
    sptr<BundleDeathRecipient> recipient(new (std::nothrow) BundleDeathRecipient(nullptr, callback));
    if (recipient == nullptr) {
        APP_LOGE("recipient is null");
        return ERR_BUNDLEMANAGER_QUICK_FIX_INTERNAL_ERROR;
    }
    bundleMgrProxy_->AsObject()->AddDeathRecipient(recipient);
    auto quickFixProxy = bundleMgrProxy_->GetQuickFixManagerProxy();
    if (quickFixProxy == nullptr) {
        APP_LOGE("quickFixProxy is null");
        return ERR_BUNDLEMANAGER_QUICK_FIX_INTERNAL_ERROR;
    }
    std::vector<std::string> destFiles;
    auto res = quickFixProxy->CopyFiles(pathVec, destFiles);
    if (res != ERR_OK) {
        APP_LOGE("Copy files failed with %{public}d.", res);
        return res;
    }
    res = quickFixProxy->DeployQuickFix(destFiles, callback, isDebug, targetPath);
    if (res != ERR_OK) {
        APP_LOGE("DeployQuickFix failed");
        return res;
    }
    return callback->GetResultCode(quickFixRes);
}

ErrCode BundleManagerShellCommand::DeleteQuickFix(const std::string &bundleName,
    std::shared_ptr<QuickFixResult> &quickFixRes) const
{
    APP_LOGD("DeleteQuickFix bundleName: %{public}s", bundleName.c_str());
    sptr<QuickFixStatusCallbackHostlmpl> callback(new (std::nothrow) QuickFixStatusCallbackHostlmpl());
    if (callback == nullptr || bundleMgrProxy_ == nullptr) {
        APP_LOGE("callback or bundleMgrProxy is null");
        return ERR_BUNDLEMANAGER_QUICK_FIX_INTERNAL_ERROR;
    }
    sptr<BundleDeathRecipient> recipient(new (std::nothrow) BundleDeathRecipient(nullptr, callback));
    if (recipient == nullptr) {
        APP_LOGE("recipient is null");
        return ERR_BUNDLEMANAGER_QUICK_FIX_INTERNAL_ERROR;
    }
    bundleMgrProxy_->AsObject()->AddDeathRecipient(recipient);
    auto quickFixProxy = bundleMgrProxy_->GetQuickFixManagerProxy();
    if (quickFixProxy == nullptr) {
        APP_LOGE("quickFixProxy is null");
        return ERR_BUNDLEMANAGER_QUICK_FIX_INTERNAL_ERROR;
    }
    auto res = quickFixProxy->DeleteQuickFix(bundleName, callback);
    if (res != ERR_OK) {
        APP_LOGE("DeleteQuickFix failed");
        return res;
    }
    return callback->GetResultCode(quickFixRes);
}

std::string BundleManagerShellCommand::GetWaringString(int32_t currentUserId, int32_t specifedUserId) const
{
    std::string res = WARNING_USER;
    size_t pos = res.find('%');
    while (pos!= std::string::npos) {
        res.replace(pos, 1, std::to_string(currentUserId));
        pos = res.find('%');
    }
    pos = res.find('$');
    while (pos!= std::string::npos) {
        res.replace(pos, 1, std::to_string(specifedUserId));
        pos = res.find('$');
    }
    return res;
}

int32_t BundleManagerShellCommand::TransformErrCode(const int32_t resultCode)
{
    if (errCodeMap_.find(resultCode) != errCodeMap_.end()) {
        return errCodeMap_.at(resultCode);
    }
    return IStatusReceiver::ERR_INSTALL_INTERNAL_ERROR;
}

ErrCode BundleManagerShellCommand::RunAsInstallPluginCommand()
{
    APP_LOGI("begin to RunAsInstallPluginCommand");
    int result = OHOS::ERR_OK;
    std::string hostBundleName;
    std::vector<std::string> pluginPaths;
    int32_t userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    int32_t option;
    while ((option = getopt_long(argc_, argv_, SHORT_OPTIONS_INSTALL_PLUGIN.c_str(),
        LONG_OPTIONS_INSTALL_PLUGIN, nullptr)) != -1) {
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        result = ParseInstallPluginCommand(option, hostBundleName, pluginPaths);
    }
    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && (pluginPaths.empty() || hostBundleName.empty())) {
            APP_LOGD("'bm install-plugin' with no bundle path option.");
            resultReceiver_.append(HELP_MSG_NO_INSTALL_PLUGIN_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_INSTALL_PLUGIN);
    } else {
        InstallPluginParam installPluginParam;
        installPluginParam.userId = userId;
        std::vector<std::string> pathVec;
        GetAbsPaths(pluginPaths, pathVec);
        int32_t installResult = bundleInstallerProxy_->InstallPlugin(hostBundleName, pathVec, installPluginParam);
        if (installResult == OHOS::ERR_OK) {
            resultReceiver_ = STRING_INSTALL_BUNDLE_OK + "\n";
        } else {
            int32_t res = TransformErrCode(installResult);
            resultReceiver_ = STRING_INSTALL_BUNDLE_NG + "\n";
            resultReceiver_.append(GetMessageFromCode(res));
        }
    }
    APP_LOGI("RunAsInstallPluginCommand end");
    return result;
}

ErrCode BundleManagerShellCommand::ParseInstallPluginCommand(int32_t option, std::string &hostBundleName,
    std::vector<std::string> &pluginPaths)
{
    int32_t result = OHOS::ERR_OK;
    if (option == '?') {
        switch (optopt) {
            case 'n': {
                // 'bm install-plugin -n' with no argument: bm install-plugin -n
                // 'bm install-plugin --host-bundle-name' with no argument: bm install-plugin --host-bundle-name
                APP_LOGD("'bm install-plugin -n' with no argument.");
                resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'p': {
                // 'bm install-plugin -p' with no argument: bm install-plugin -p
                // 'bm install-plugin --plugin-path' with no argument: bm install-plugin --plugin-path
                APP_LOGD("'bm install-plugin -p' with no argument.");
                resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                // 'bm install-plugin' with an unknown option: bm dump-shared -x
                // 'bm install-plugin' with an unknown option: bm dump-shared -xxx
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_.append(unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    } else {
        switch (option) {
            case 'h': {
                // 'bm install-plugin -h'
                // 'bm install-plugin --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm install-plugin -n xxx'
                // 'bm install-plugin --host-bundle-name xxx'
                hostBundleName = optarg;
                break;
            }
            case 'p': {
                // 'bm install-plugin -p xxx'
                // 'bm install-plugin --plugin-path xxx'
                APP_LOGD("'bm install-plugin %{public}s'", argv_[optind - 1]);
                if (GetPluginPath(optarg, pluginPaths) != OHOS::ERR_OK) {
                    resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
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
    return result;
}
ErrCode BundleManagerShellCommand::RunAsUninstallPluginCommand()
{
    APP_LOGI("begin to RunAsUninstallPluginCommand");
    int result = OHOS::ERR_OK;
    std::string hostBundleName;
    std::string pluginBundleName;
    int32_t userId = BundleCommandCommon::GetCurrentUserId(Constants::UNSPECIFIED_USERID);
    int32_t option;
    while ((option = getopt_long(argc_, argv_, SHORT_OPTIONS_UNINSTALL_PLUGIN.c_str(),
        LONG_OPTIONS_UNINSTALL_PLUGIN, nullptr)) != -1) {
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        result = ParseUninstallPluginCommand(option, hostBundleName, pluginBundleName);
    }
    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && (pluginBundleName.empty() || hostBundleName.empty())) {
            APP_LOGD("'bm uninstall-plugin' with no bundle path option.");
            resultReceiver_.append(HELP_MSG_NO_UNINSTALL_PLUGIN_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_UNINSTALL_PLUGIN);
    } else {
        InstallPluginParam installPluginParam;
        installPluginParam.userId = userId;
        int32_t installResult =
            bundleInstallerProxy_->UninstallPlugin(hostBundleName, pluginBundleName, installPluginParam);
        if (installResult == OHOS::ERR_OK) {
            resultReceiver_ = STRING_UNINSTALL_BUNDLE_OK + "\n";
        } else {
            int32_t res = TransformErrCode(installResult);
            resultReceiver_ = STRING_UNINSTALL_BUNDLE_NG + "\n";
            resultReceiver_.append(GetMessageFromCode(res));
        }
    }
    APP_LOGI("RunAsUninstallPluginCommand end");
    return result;
}

ErrCode BundleManagerShellCommand::ParseUninstallPluginCommand(int32_t option, std::string &hostBundleName,
    std::string &pluginBundleName)
{
    int32_t result = OHOS::ERR_OK;
    if (option == '?') {
        switch (optopt) {
            case 'n': {
                // 'bm uninstall-plugin -n' with no argument: bm uninstall-plugin -n
                // 'bm uninstall-plugin --host-bundle-name' with no argument: bm uninstall-plugin --host-bundle-name
                APP_LOGD("'bm uninstall-plugin -n' with no argument.");
                resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'p': {
                // 'bm uninstall-plugin -p' with no argument: bm uninstall-plugin -p
                // 'bm uninstall-plugin --plugin-bundle-name' with no argument: bm uninstall-plugin --plugin-bundle-name
                APP_LOGD("'bm uninstall-plugin -p' with no argument.");
                resultReceiver_.append(STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                // 'bm uninstall-plugin' with an unknown option: bm uninstall-plugin -x
                // 'bm uninstall-plugin' with an unknown option: bm uninstall-plugin -xxx
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_.append(unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    } else {
        switch (option) {
            case 'h': {
                // 'bm uninstall-plugin -h'
                // 'bm uninstall-plugin --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                // 'bm uninstall-plugin -n xxx'
                // 'bm uninstall-plugin --host-bundle-name xxx'
                hostBundleName = optarg;
                break;
            }
            case 'p': {
                // 'bm uninstall-plugin -p xxx'
                // 'bm uninstall-plugin --plugin-bundle-name xxx'
                APP_LOGD("'bm uninstall-plugin %{public}s'", argv_[optind - 1]);
                pluginBundleName = optarg;
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
}  // namespace AppExecFwk
}  // namespace OHOS
