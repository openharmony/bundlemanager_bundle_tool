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
#include "cJSON.h"
#include "status_receiver_impl.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t INDEX_OFFSET = 2;

const std::string UNINSTALL_OPTIONS = "hn:kv:s";
const struct option UNINSTALL_LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundleName", required_argument, nullptr, 'n'},
    {"keepData", no_argument, nullptr, 'k'},
    {"version", required_argument, nullptr, 'v'},
    {"shared", no_argument, nullptr, 's'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_DUMP = "hn:aisd:gl";
const struct option LONG_OPTIONS_DUMP[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundleName", required_argument, nullptr, 'n'},
    {"all", no_argument, nullptr, 'a'},
    {"bundleInfo", no_argument, nullptr, 'i'},
    {"shortcutInfo", no_argument, nullptr, 's'},
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

const std::string CLEAN_SHORT_OPTIONS = "hn:cdi:";
const struct option CLEAN_LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundleName", required_argument, nullptr, 'n'},
    {"cache", no_argument, nullptr, 'c'},
    {"data", no_argument, nullptr, 'd'},
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

namespace {
cJSON* SharedBundleInfoToJson(const SharedBundleInfo &info)
{
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "name", info.name.c_str());
    cJSON_AddNumberToObject(obj, "compatiblePolicy", static_cast<int>(info.compatiblePolicy));
    cJSON *moduleArray = cJSON_CreateArray();
    for (const auto &module : info.sharedModuleInfos) {
        cJSON *mod = cJSON_CreateObject();
        cJSON_AddStringToObject(mod, "name", module.name.c_str());
        cJSON_AddNumberToObject(mod, "versionCode", module.versionCode);
        cJSON_AddStringToObject(mod, "versionName", module.versionName.c_str());
        cJSON_AddStringToObject(mod, "description", module.description.c_str());
        cJSON_AddNumberToObject(mod, "descriptionId", module.descriptionId);
        cJSON_AddNumberToObject(mod, "aotCompileStatus", static_cast<int>(module.aotCompileStatus));
        cJSON_AddBoolToObject(mod, "compressNativeLibs", module.compressNativeLibs);
        cJSON_AddStringToObject(mod, "hapPath", module.hapPath.c_str());
        cJSON_AddStringToObject(mod, "cpuAbi", module.cpuAbi.c_str());
        cJSON_AddStringToObject(mod, "nativeLibraryPath", module.nativeLibraryPath.c_str());
        cJSON_AddStringToObject(mod, "moduleArkTSMode", module.moduleArkTSMode.c_str());
        cJSON *fileNames = cJSON_CreateArray();
        for (const auto &fn : module.nativeLibraryFileNames) {
            cJSON_AddItemToArray(fileNames, cJSON_CreateString(fn.c_str()));
        }
        cJSON_AddItemToObject(mod, "nativeLibraryFileNames", fileNames);
        cJSON *libDirs = cJSON_CreateArray();
        for (const auto &dir : module.librarySupportDirectory) {
            cJSON_AddItemToArray(libDirs, cJSON_CreateString(dir.c_str()));
        }
        cJSON_AddItemToObject(mod, "librarySupportDirectory", libDirs);
        cJSON_AddItemToArray(moduleArray, mod);
    }
    cJSON_AddItemToObject(obj, "sharedModuleInfos", moduleArray);
    return obj;
}
}  // namespace

BundleManagerShellCommand::BundleManagerShellCommand(int argc, char *argv[])
    : ShellCommand(argc, argv, TOOL_NAME)
{}

std::string BundleManagerShellCommand::CreateSuccessResult(const std::string &data) const
{
    cJSON *result = cJSON_CreateObject();
    if (result == nullptr) {
        return "";
    }
    cJSON_AddStringToObject(result, "type", "result");
    cJSON_AddStringToObject(result, "status", "success");
    if (data.empty()) {
        cJSON_AddItemToObject(result, "data", cJSON_CreateObject());
    } else {
        cJSON *parsed = cJSON_Parse(data.c_str());
        if (parsed != nullptr) {
            cJSON_AddItemToObject(result, "data", parsed);
        } else {
            cJSON *wrapper = cJSON_CreateObject();
            cJSON_AddStringToObject(wrapper, "content", data.c_str());
            cJSON_AddItemToObject(result, "data", wrapper);
        }
    }
    char *output = cJSON_PrintUnformatted(result);
    std::string ret(output);
    cJSON_Delete(result);
    cJSON_free(output);
    return ret;
}

std::string BundleManagerShellCommand::CreateErrorResult(int32_t code,
    const std::string &message, const std::string &suggestion) const
{
    cJSON *result = cJSON_CreateObject();
    if (result == nullptr) {
        return "";
    }
    cJSON_AddStringToObject(result, "type", "result");
    cJSON_AddStringToObject(result, "status", "failed");
    cJSON_AddStringToObject(result, "errCode", ErrorCodeUtils::GetErrorCodeString(code).c_str());
    std::string errMsg = message;
    std::string codeMessage = GetMessageFromCode(code);
    if (!codeMessage.empty()) {
        errMsg += "\n" + codeMessage;
    }
    cJSON_AddStringToObject(result, "errMsg", errMsg.c_str());
    cJSON_AddStringToObject(result, "suggestion", suggestion.c_str());
    char *output = cJSON_PrintUnformatted(result);
    std::string ret(output);
    cJSON_Delete(result);
    cJSON_free(output);
    return ret;
}

std::string BundleManagerShellCommand::CreateErrorResult(const std::string &errCode,
    const std::string &message, const std::string &suggestion) const
{
    cJSON *result = cJSON_CreateObject();
    if (result == nullptr) {
        return "";
    }
    cJSON_AddStringToObject(result, "type", "result");
    cJSON_AddStringToObject(result, "status", "failed");
    cJSON_AddStringToObject(result, "errCode", errCode.c_str());
    cJSON_AddStringToObject(result, "errMsg", message.c_str());
    cJSON_AddStringToObject(result, "suggestion", suggestion.c_str());
    char *output = cJSON_PrintUnformatted(result);
    std::string ret(output);
    cJSON_Delete(result);
    cJSON_free(output);
    return ret;
}

ErrCode BundleManagerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"--help", [this] { return this->RunAsHelpCommand(); } },
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
    resultReceiver_ = HELP_MSG;
    return OHOS::ERR_OK;
}

ErrCode BundleManagerShellCommand::RunAsUninstallCommand()
{
    APP_LOGI("begin to RunAsUninstallCommand");

    // Return error when no arguments provided
    if (argc_ <= 2) {
        APP_LOGD("'ohos-bm uninstall' with no option.");
        resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR, HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    // uninstall 命令需要 installer proxy
    if (InitInstaller() != OHOS::ERR_OK) {
        resultReceiver_ = CreateErrorResult(ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR,
            "error: failed to connect to bundle installer service.");
        return OHOS::ERR_INVALID_VALUE;
    }

    int result = OHOS::ERR_OK;
    int counter = 0;
    std::string bundleName = "";
    int32_t userId = BundleCommandCommon::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid());
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
                resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR, HELP_MSG_NO_OPTION);
                result = OHOS::ERR_INVALID_VALUE;
            }

        if (option == '?') {
            switch (optopt) {
                case 'n': {
                    APP_LOGD("'ohos-bm uninstall -n' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
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
                        IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'ohos-bm uninstall' with an unknown option.");
                    resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR, unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm uninstall %{public}s'", argv_[optind - 1]);
                resultReceiver_ = HELP_MSG_UNINSTALL;
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                APP_LOGD("'ohos-bm uninstall %{public}s %{public}s'",
                    argv_[optind - INDEX_OFFSET], optarg);
                bundleName = optarg;
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
                        IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
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
            resultReceiver_ = CreateErrorResult(IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR, HELP_MSG_UNINSTALL);
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
            resultReceiver_ = CreateSuccessResult();
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
        int32_t uninstallResult = UninstallOperation(bundleName, installParam);
        if (uninstallResult == OHOS::ERR_OK) {
            resultReceiver_ = CreateSuccessResult();
        } else {
            resultReceiver_ = CreateErrorResult(uninstallResult,
                STRING_UNINSTALL_BUNDLE_NG);
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
        resultReceiver_ = CreateErrorResult(ERR_DUMP_PARAM_ERROR, HELP_MSG_NO_OPTION);
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
    int32_t userId = BundleCommandCommon::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid());
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
                        ERR_DUMP_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'd': {
                    APP_LOGD("'ohos-bm dump -d' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        ERR_DUMP_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'ohos-bm dump' with an unknown option.");
                    resultReceiver_ = CreateErrorResult(ERR_DUMP_PARAM_ERROR, unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm dump %{public}s'", argv_[optind - 1]);
                resultReceiver_ = HELP_MSG_DUMP;
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
                ERR_DUMP_PARAM_ERROR, HELP_MSG_NO_BUNDLE_NAME_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
        if ((resultReceiver_ == "") && bundleDumpDistributedBundleInfo && (bundleName.size() == 0)) {
            APP_LOGD("'ohos-bm dump -d' with no bundle name option.");
            resultReceiver_ = CreateErrorResult(
                ERR_DUMP_PARAM_ERROR, HELP_MSG_NO_BUNDLE_NAME_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        if (resultReceiver_ == "") {
            resultReceiver_ = CreateErrorResult(ERR_DUMP_PARAM_ERROR, HELP_MSG_DUMP);
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
            resultReceiver_ = CreateErrorResult(
                ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR, HELP_MSG_DUMP_FAILED);
        } else {
            resultReceiver_ = CreateSuccessResult(dumpResults);
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
        resultReceiver_ = CreateErrorResult(ERR_DUMP_DEPENDENCIES_PARAM_ERROR, HELP_MSG_NO_OPTION);
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
            resultReceiver_ = CreateErrorResult(ERR_DUMP_DEPENDENCIES_PARAM_ERROR, HELP_MSG_NO_REMOVABLE_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
    }
    if (result != OHOS::ERR_OK) {
        if (resultReceiver_ == "") {
            resultReceiver_ = CreateErrorResult(
                ERR_DUMP_DEPENDENCIES_PARAM_ERROR, HELP_MSG_DUMP_SHARED_DEPENDENCIES);
        }
    } else {
        std::vector<Dependency> dependencies;
        ErrCode ret = bundleMgrProxy_->GetSharedDependencies(bundleName, moduleName, dependencies);
        if (ret != ERR_OK) {
            APP_LOGE("dump shared dependencies failed due to errcode %{public}d", ret);
            resultReceiver_ = CreateErrorResult(static_cast<int32_t>(ret), HELP_MSG_DUMP_FAILED);
            result = ret;
        } else {
            cJSON *jsonResult = cJSON_CreateObject();
            cJSON *depArray = cJSON_CreateArray();
            for (const auto &dep : dependencies) {
                cJSON *item = cJSON_CreateObject();
                cJSON_AddStringToObject(item, Constants::BUNDLE_NAME, dep.bundleName.c_str());
                cJSON_AddStringToObject(item, Constants::MODULE_NAME, dep.moduleName.c_str());
                cJSON_AddNumberToObject(item, "versionCode", dep.versionCode);
                cJSON_AddItemToArray(depArray, item);
            }
            cJSON_AddItemToObject(jsonResult, DEPENDENCIES.c_str(), depArray);
            char *output = cJSON_PrintBuffered(jsonResult, Constants::DUMP_INDENT, 1);
            resultReceiver_ = CreateSuccessResult(std::string(output));
            cJSON_Delete(jsonResult);
            cJSON_free(output);
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
                    ERR_DUMP_DEPENDENCIES_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'm': {
                resultReceiver_ = CreateErrorResult(
                    ERR_DUMP_DEPENDENCIES_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_ = CreateErrorResult(ERR_DUMP_DEPENDENCIES_PARAM_ERROR, unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    } else {
        switch (option) {
            case 'h': {
                resultReceiver_ = HELP_MSG_DUMP_SHARED_DEPENDENCIES;
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
        resultReceiver_ = CreateErrorResult(ERR_DUMP_SHARED_PARAM_ERROR, HELP_MSG_NO_OPTION);
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
            resultReceiver_ = CreateErrorResult(ERR_DUMP_SHARED_PARAM_ERROR, HELP_MSG_DUMP_SHARED);
        }
    } else if (dumpSharedAll) {
        std::vector<SharedBundleInfo> sharedBundleInfos;
        ErrCode ret = bundleMgrProxy_->GetAllSharedBundleInfo(sharedBundleInfos);
        if (ret != ERR_OK) {
            APP_LOGE("dump-shared all failed due to errcode %{public}d", ret);
            resultReceiver_ = CreateErrorResult(static_cast<int32_t>(ret), HELP_MSG_DUMP_FAILED);
            result = ret;
        } else {
            std::vector<std::string> sharedBundleNames;
            for (const auto& item : sharedBundleInfos) {
                sharedBundleNames.push_back(item.name);
            }
            cJSON *jsonResult = cJSON_CreateObject();
            cJSON *nameArray = cJSON_CreateArray();
            for (const auto &name : sharedBundleNames) {
                cJSON_AddItemToArray(nameArray, cJSON_CreateString(name.c_str()));
            }
            cJSON_AddItemToObject(jsonResult, SHARED_BUNDLE_INFO.c_str(), nameArray);
            char *output = cJSON_PrintBuffered(jsonResult, Constants::DUMP_INDENT, 1);
            resultReceiver_ = CreateSuccessResult(std::string(output));
            cJSON_Delete(jsonResult);
            cJSON_free(output);
        }
    } else {
        if ((resultReceiver_ == "") && (bundleName.size() == 0)) {
            resultReceiver_ = CreateErrorResult(ERR_DUMP_SHARED_PARAM_ERROR, HELP_MSG_NO_REMOVABLE_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
            return result;
        }
        SharedBundleInfo sharedBundleInfo;
        ErrCode ret = bundleMgrProxy_->GetSharedBundleInfoBySelf(bundleName, sharedBundleInfo);
        if (ret != ERR_OK) {
            APP_LOGE("dump-shared failed due to errcode %{public}d", ret);
            resultReceiver_ = CreateErrorResult(static_cast<int32_t>(ret), HELP_MSG_DUMP_FAILED);
            result = ret;
        } else {
            cJSON *jsonResult = cJSON_CreateObject();
            cJSON_AddItemToObject(jsonResult, SHARED_BUNDLE_INFO.c_str(), SharedBundleInfoToJson(sharedBundleInfo));
            char *output = cJSON_PrintBuffered(jsonResult, Constants::DUMP_INDENT, 1);
            resultReceiver_ = CreateSuccessResult(std::string(output));
            cJSON_Delete(jsonResult);
            cJSON_free(output);
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
                    ERR_DUMP_SHARED_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_ = CreateErrorResult(ERR_DUMP_SHARED_PARAM_ERROR, unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    } else {
        switch (option) {
            case 'h': {
                resultReceiver_ = HELP_MSG_DUMP_SHARED;
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
        resultReceiver_ = CreateErrorResult(ERR_CLEAN_PARAM_ERROR, HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }

    int32_t counter = 0;
    int32_t userId = BundleCommandCommon::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid());
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
                        ERR_CLEAN_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'i': {
                    APP_LOGD("'ohos-bm clean -i' with no argument.");
                    resultReceiver_ = CreateErrorResult(
                        ERR_CLEAN_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    APP_LOGD("'ohos-bm clean' with an unknown option.");
                    resultReceiver_ = CreateErrorResult(ERR_CLEAN_PARAM_ERROR, unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm clean %{public}s'", argv_[optind - 1]);
                resultReceiver_ = HELP_MSG_CLEAN;
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
            case 'i': {
                if (!OHOS::StrToInt(optarg, appIndex) || (appIndex < 0 || appIndex > INITIAL_SANDBOX_APP_INDEX)) {
                    APP_LOGE("ohos-bm clean with error appIndex %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        ERR_CLEAN_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
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
                ERR_CLEAN_PARAM_ERROR, HELP_MSG_NO_BUNDLE_NAME_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
        if (!cleanCache && !cleanData) {
            APP_LOGD("'ohos-bm clean' with no --cache or --data option.");
            resultReceiver_ = CreateErrorResult(
                ERR_CLEAN_PARAM_ERROR, HELP_MSG_NO_DATA_OR_CACHE_OPTION);
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        if (resultReceiver_ == "") {
            resultReceiver_ = CreateErrorResult(ERR_CLEAN_PARAM_ERROR, HELP_MSG_CLEAN);
        }
    } else {
        cJSON *cleanResult = cJSON_CreateObject();
        if (cleanCache) {
            if (CleanBundleCacheFilesOperation(bundleName, userId, appIndex)) {
                cJSON_AddStringToObject(cleanResult, "cache", STRING_CLEAN_CACHE_BUNDLE_OK.c_str());
                char *output = cJSON_PrintUnformatted(cleanResult);
                resultReceiver_ = CreateSuccessResult(std::string(output));
                cJSON_free(output);
            } else {
                resultReceiver_ = CreateErrorResult(
                    ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR, STRING_CLEAN_CACHE_BUNDLE_NG);
            }
        }
        if (cleanData) {
            if (CleanBundleDataFilesOperation(bundleName, userId, appIndex)) {
                cJSON_AddStringToObject(cleanResult, "data", STRING_CLEAN_DATA_BUNDLE_OK.c_str());
                char *output = cJSON_PrintUnformatted(cleanResult);
                resultReceiver_ = CreateSuccessResult(std::string(output));
                cJSON_free(output);
            } else {
                resultReceiver_ = CreateErrorResult(
                    ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR, STRING_CLEAN_DATA_BUNDLE_NG);
            }
        }
        cJSON_Delete(cleanResult);
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
    ErrCode cleanRetAms = appMgrClient->ClearUpApplicationData(bundleName, appIndex, userId);
    return cleanRetAms == ERR_OK;
}

// Common helper methods

int32_t BundleManagerShellCommand::UninstallOperation(
    const std::string &bundleName, InstallParam &installParam) const
{
    sptr<StatusReceiverImpl> statusReceiver(new (std::nothrow) StatusReceiverImpl());
    if (statusReceiver == nullptr) {
        APP_LOGE("statusReceiver is null");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    APP_LOGD("bundleName: %{public}s", bundleName.c_str());

    sptr<BundleDeathRecipient> recipient(new (std::nothrow) BundleDeathRecipient(statusReceiver));
    if (recipient == nullptr) {
        APP_LOGE("recipient is null");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }
    bundleInstallerProxy_->AsObject()->AddDeathRecipient(recipient);
    bundleInstallerProxy_->Uninstall(bundleName, installParam, statusReceiver);

    return statusReceiver->GetResultCode();
}

int32_t BundleManagerShellCommand::UninstallSharedOperation(const UninstallParam &uninstallParam) const
{
    sptr<StatusReceiverImpl> statusReceiver(new (std::nothrow) StatusReceiverImpl());
    if (statusReceiver == nullptr) {
        APP_LOGE("statusReceiver is null");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    sptr<BundleDeathRecipient> recipient(new (std::nothrow) BundleDeathRecipient(statusReceiver));
    if (recipient == nullptr) {
        APP_LOGE("recipient is null");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }
    bundleInstallerProxy_->AsObject()->AddDeathRecipient(recipient);

    bundleInstallerProxy_->Uninstall(uninstallParam, statusReceiver);
    return statusReceiver->GetResultCode();
}
// parse integer parameters
ErrCode BundleManagerShellCommand::ParseParamInteger(std::map<std::string, int>& pi)
{
    std::string sarg(optarg);
    if (!sarg.empty() && sarg.front() == '\'') {
        sarg.erase(0, 1);
    }
    if (!sarg.empty() && sarg.back() == '\'') {
        sarg.pop_back();
    }
    cJSON *paramObj = cJSON_Parse(sarg.c_str());
    if (paramObj == nullptr || !cJSON_IsObject(paramObj)) {
        cJSON_Delete(paramObj);
        return OHOS::ERR_INVALID_VALUE;
    }
    cJSON *item = nullptr;
    cJSON_ArrayForEach(item, paramObj) {
        if (item->string == nullptr || strlen(item->string) == 0) {
            cJSON_Delete(paramObj);
            return OHOS::ERR_INVALID_VALUE;
        }
        if (!cJSON_IsNumber(item)) {
            cJSON_Delete(paramObj);
            return OHOS::ERR_INVALID_VALUE;
        }
        pi[item->string] = static_cast<int>(item->valuedouble);
    }
    cJSON_Delete(paramObj);
    return OHOS::ERR_OK;
}

// parse bool parameters
ErrCode BundleManagerShellCommand::ParseParamBool(std::map<std::string, bool>& pb)
{
    std::string sarg(optarg);
    if (!sarg.empty() && sarg.front() == '\'') {
        sarg.erase(0, 1);
    }
    if (!sarg.empty() && sarg.back() == '\'') {
        sarg.pop_back();
    }
    cJSON *paramObj = cJSON_Parse(sarg.c_str());
    if (paramObj == nullptr || !cJSON_IsObject(paramObj)) {
        cJSON_Delete(paramObj);
        return OHOS::ERR_INVALID_VALUE;
    }
    cJSON *item = nullptr;
    cJSON_ArrayForEach(item, paramObj) {
        if (item->string == nullptr || strlen(item->string) == 0) {
            cJSON_Delete(paramObj);
            return OHOS::ERR_INVALID_VALUE;
        }
        if (!cJSON_IsBool(item)) {
            cJSON_Delete(paramObj);
            return OHOS::ERR_INVALID_VALUE;
        }
        pb[item->string] = cJSON_IsTrue(item);
    }
    cJSON_Delete(paramObj);
    return OHOS::ERR_OK;
}

// parse string parameters
ErrCode BundleManagerShellCommand::ParseParamString(std::map<std::string, std::string>& ps)
{
    std::string sarg(optarg);
    if (!sarg.empty() && sarg.front() == '\'') {
        sarg.erase(0, 1);
    }
    if (!sarg.empty() && sarg.back() == '\'') {
        sarg.pop_back();
    }
    cJSON *paramObj = cJSON_Parse(sarg.c_str());
    if (paramObj == nullptr || !cJSON_IsObject(paramObj)) {
        cJSON_Delete(paramObj);
        return OHOS::ERR_INVALID_VALUE;
    }
    cJSON *item = nullptr;
    cJSON_ArrayForEach(item, paramObj) {
        if (item->string == nullptr || strlen(item->string) == 0) {
            cJSON_Delete(paramObj);
            return OHOS::ERR_INVALID_VALUE;
        }
        if (!cJSON_IsString(item)) {
            cJSON_Delete(paramObj);
            return OHOS::ERR_INVALID_VALUE;
        }
        ps[item->string] = item->valuestring;
    }
    cJSON_Delete(paramObj);
    return OHOS::ERR_OK;
}


ErrCode BundleManagerShellCommand::RunAsSetDisposedRuleCommand()
{
    APP_LOGI("begin to RunAsSetDisposedRuleCommand");
    if (argc_ <= 2) {
        resultReceiver_ = CreateErrorResult(ERR_SET_DISPOSED_RULE_PARAM_ERROR, HELP_MSG_NO_OPTION);
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
    std::map<std::string, int> parametersInteger;
    std::map<std::string, std::string> parametersString;
    std::map<std::string, bool> parametersBool;
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
            break;
        }

        if (option == '?') {
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            resultReceiver_ = CreateErrorResult(ERR_SET_DISPOSED_RULE_PARAM_ERROR, unknownOptionMsg);
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }

        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm set-disposed-rule %{public}s'", argv_[optind - 1]);
                resultReceiver_ = HELP_MSG_SET_DISPOSED_RULE;
                return OHOS::ERR_OK;
            }
            case OPTION_APP_ID: {
                appId = optarg;
                hasAppId = true;
                break;
            }
            case OPTION_APP_INDEX: {
                if (!OHOS::StrToInt(optarg, appIndex)) {
                    APP_LOGE("ohos-bm set-disposed-rule with error appIndex %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        ERR_SET_DISPOSED_RULE_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            case OPTION_PRIORITY: {
                if (!OHOS::StrToInt(optarg, disposedRule.priority)) {
                    APP_LOGE("ohos-bm set-disposed-rule with error priority %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(
                        ERR_SET_DISPOSED_RULE_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
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
                    resultReceiver_ = CreateErrorResult(
                        ERR_SET_DISPOSED_RULE_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
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
                    resultReceiver_ = CreateErrorResult(
                        ERR_SET_DISPOSED_RULE_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
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
                    resultReceiver_ = CreateErrorResult(
                        ERR_SET_DISPOSED_RULE_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
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
                    resultReceiver_ = CreateErrorResult(ERR_SET_DISPOSED_RULE_PARAM_ERROR,
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
                ErrCode res = ParseParamString(parametersString);
                if (res != OHOS::ERR_OK) {
                    resultReceiver_ = CreateErrorResult(ERR_SET_DISPOSED_RULE_PARAM_ERROR,
                        "error: --wantParamsStrings requires key and value.");
                    return OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            case OPTION_WANT_PI: {
                ErrCode res = ParseParamInteger(parametersInteger);
                if (res != OHOS::ERR_OK) {
                    APP_LOGE("ohos-bm set-disposed-rule --wantParamsInts with error value");
                    resultReceiver_ = CreateErrorResult(
                        ERR_SET_DISPOSED_RULE_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            case OPTION_WANT_PB: {
                ErrCode res = ParseParamBool(parametersBool);
                if (res != OHOS::ERR_OK) {
                    APP_LOGE("ohos-bm set-disposed-rule --wantParamsInts with error value");
                    resultReceiver_ = CreateErrorResult(ERR_SET_DISPOSED_RULE_PARAM_ERROR,
                        "error: --wantParamsBools requires key and value.");
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
        if (!hasAppId || !hasPriority || !hasComponentType ||
            !hasDisposedType || !hasControlType ||
            !hasWantBundleName || !hasWantAbilityName) {
            APP_LOGD("'ohos-bm set-disposed-rule' missing required options.");
            resultReceiver_ = CreateErrorResult(ERR_SET_DISPOSED_RULE_PARAM_ERROR,
                "error: --appId, --priority, --componentType, "
                "--disposedType, --controlType, --wantBundleName, "
                "--wantAbilityName are required.");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_ = CreateErrorResult(ERR_SET_DISPOSED_RULE_PARAM_ERROR, HELP_MSG_SET_DISPOSED_RULE);
        return result;
    }

    // Get userId from uid
    int32_t userId = BundleCommandCommon::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid());
    if (userId == Constants::DEFAULT_USERID) {
        APP_LOGE("userId is 0, forbidden to call set-disposed-rule");
        resultReceiver_ = CreateErrorResult(
            ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR, STRING_USER_ID_INVALID);
        return OHOS::ERR_INVALID_VALUE;
    }

    // Build Want (want-bundle-name, want-module-name, want-ability-name are required)
    auto want = std::make_shared<AAFwk::Want>();
    ElementName elementName("", wantBundleName, wantAbilityName, wantModuleName);
    want->SetElement(elementName);
    for (auto& [key, value] : parametersString) {
        want->SetParam(key, value);
    }
    for (auto& [key, value] : parametersInteger) {
        want->SetParam(key, value);
    }
    for (auto& [key, value] : parametersBool) {
        want->SetParam(key, value);
    }
    disposedRule.want = want;

    // Call IPC interface
    if (InitAppControlProxy() != OHOS::ERR_OK && appControlProxy_ == nullptr) {
        APP_LOGE("appControlProxy_ is null");
        resultReceiver_ = CreateErrorResult(
            ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR, "error: failed to get app control proxy.");
        return OHOS::ERR_INVALID_VALUE;
    }

    ErrCode res = appControlProxy_->SetDisposedRuleForCloneApp(appId, disposedRule, appIndex, userId);
    if (res == OHOS::ERR_OK) {
        resultReceiver_ = CreateSuccessResult();
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
        resultReceiver_ = CreateErrorResult(ERR_SET_DELETE_RULE_PARAM_ERROR, HELP_MSG_NO_OPTION);
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
            break;
        }

        if (option == '?') {
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            resultReceiver_ = CreateErrorResult(ERR_SET_DELETE_RULE_PARAM_ERROR, unknownOptionMsg);
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }

        switch (option) {
            case 'h': {
                APP_LOGD("'ohos-bm delete-disposed-rule %{public}s'", argv_[optind - 1]);
                resultReceiver_ = HELP_MSG_DELETE_DISPOSED_RULE;
                return OHOS::ERR_OK;
            }
            case OPTION_APP_ID: {
                appId = optarg;
                hasAppId = true;
                break;
            }
            case OPTION_APP_INDEX: {
                if (!OHOS::StrToInt(optarg, appIndex)) {
                    APP_LOGE("ohos-bm delete-disposed-rule with error appIndex %{private}s", optarg);
                    resultReceiver_ = CreateErrorResult(ERR_SET_DELETE_RULE_PARAM_ERROR, STRING_REQUIRE_CORRECT_VALUE);
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
            resultReceiver_ = CreateErrorResult(ERR_SET_DELETE_RULE_PARAM_ERROR,
                "error: --appId is required.");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_ = CreateErrorResult(ERR_SET_DELETE_RULE_PARAM_ERROR, HELP_MSG_DELETE_DISPOSED_RULE);
        return result;
    }

    // Get userId from uid
    int32_t userId = BundleCommandCommon::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid());
    if (userId == Constants::DEFAULT_USERID) {
        APP_LOGE("userId is 0, forbidden to call delete-disposed-rule");
        resultReceiver_ = CreateErrorResult(
            ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR, STRING_USER_ID_INVALID);
        return OHOS::ERR_INVALID_VALUE;
    }

    // Call IPC interface
    if (InitAppControlProxy() != OHOS::ERR_OK && appControlProxy_ == nullptr) {
        APP_LOGE("appControlProxy_ is null");
        resultReceiver_ = CreateErrorResult(
            ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR, "error: failed to get app control proxy.");
        return OHOS::ERR_INVALID_VALUE;
    }

    ErrCode res = appControlProxy_->DeleteDisposedRuleForCloneApp(appId, appIndex, userId);
    if (res == OHOS::ERR_OK) {
        resultReceiver_ = CreateSuccessResult();
    } else {
        resultReceiver_ = CreateErrorResult(static_cast<int32_t>(res), STRING_DELETE_DISPOSED_RULE_NG);
    }

    APP_LOGI("end");
    return res;
}

}  // namespace AppExecFwk
}  // namespace OHOS
