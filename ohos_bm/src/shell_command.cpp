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

#include "shell_command.h"

#include <getopt.h>
#include "app_log_wrapper.h"
#include "ipc_skeleton.h"
#include "privacy_kit.h"

namespace OHOS {
namespace AppExecFwk {

namespace {
const std::map<std::string, std::string> SUBCOMMAND_PERMISSION_MAP = {
    {"uninstall", "ohos.permission.cli.UNINSTALL_BUNDLE"},
    {"dump", "ohos.permission.cli.GET_BUNDLE_INFO_PRIVILEGED"},
    {"dump-dependencies", "ohos.permission.cli.GET_BUNDLE_INFO_PRIVILEGED"},
    {"dump-shared", "ohos.permission.cli.GET_BUNDLE_INFO_PRIVILEGED"},
    {"clean", "ohos.permission.cli.REMOVE_BUNDLE_DATA_AND_CACHE_FILES"},
    {"set-disposed-rule", "ohos.permission.cli.MANAGE_DISPOSED_APP_STATUS"},
    {"delete-disposed-rule", "ohos.permission.cli.MANAGE_DISPOSED_APP_STATUS"},
};
} // namespace

void ShellCommand::ReportPermissionUsedRecord(bool success)
{
    auto it = SUBCOMMAND_PERMISSION_MAP.find(cmd_);
    if (it == SUBCOMMAND_PERMISSION_MAP.end()) {
        return;
    }
    auto callerToken = IPCSkeleton::GetSelfTokenID();
    int32_t successCount = success ? 1 : 0;
    int32_t failCount = success ? 0 : 1;
    int32_t ret = Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(
        callerToken, it->second, successCount, failCount);
    if (ret != 0) {
        APP_LOGE("AddPermissionUsedRecord failed, ret = %{public}d", ret);
    }
}

ShellCommand::ShellCommand(int argc, char *argv[], std::string name)
{
    opterr = 0;
    argc_ = argc;
    argv_ = argv;
    name_ = name;

    if (argc < MIN_ARGUMENT_NUMBER) {
        cmd_ = "--help";
        return;
    }
    cmd_ = argv[1];
    for (int i = 2; i < argc; i++) {
        argList_.push_back(argv[i]);
    }
}

ShellCommand::~ShellCommand()
{}

ErrCode ShellCommand::OnCommand()
{
    int result = OHOS::ERR_OK;

    auto respond = commandMap_[cmd_];
    if (respond == nullptr) {
        resultReceiver_.append(GetCommandErrorMsg());
        respond = commandMap_["--help"];
    }

    // --help no need Init()
    if (cmd_ == "--help" || Init() == OHOS::ERR_OK) {
        ErrCode result = respond();
        APP_LOGI("cmd_: %{public}s, result: %{public}d", cmd_.c_str(), result);
        ReportPermissionUsedRecord(result == ERR_OK);
    } else {
        result = OHOS::ERR_INVALID_VALUE;
        if (resultReceiver_ == "") {
            resultReceiver_ = "error: failed to connect to bundle manager service.\n";
        }
    }

    return result;
}

std::string ShellCommand::ExecCommand()
{
    int result = CreateCommandMap();
    if (result != OHOS::ERR_OK) {
        APP_LOGE("failed to create command map.\n");
    }

    result = CreateMessageMap();
    if (result != OHOS::ERR_OK) {
        APP_LOGE("failed to create message map.\n");
    }

    result = OnCommand();
    if (result != OHOS::ERR_OK) {
        APP_LOGE("failed to execute your command.\n");
        if (resultReceiver_ == "") {
            resultReceiver_ = "error: failed to execute your command.\n";
        }
    }
    return resultReceiver_;
}

std::string ShellCommand::GetCommandErrorMsg() const
{
    std::string commandErrorMsg =
        name_ + ": '" + cmd_ + "' is not a valid " + name_ + " command. See '" + name_ + " help'.\n";

    return commandErrorMsg;
}

std::string ShellCommand::GetUnknownOptionMsg(std::string &unknownOption) const
{
    std::string result = "";

    if (optind < 0 || optind > argc_) {
        return result;
    }

    result.append("error: unknown option");
    result.append(".\n");

    return result;
}

std::string ShellCommand::GetMessageFromCode(const int32_t code) const
{
    APP_LOGI("[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    APP_LOGI("code = %{public}d", code);

    std::string result = "";
    if (messageMap_.find(code) != messageMap_.end()) {
        std::string message = messageMap_.at(code);
        if (message.size() != 0) {
            result.append(STRING_CODE + std::to_string(code) + "\n");
            result.append(message + "\n");
        }
    }

    APP_LOGI("result = %{public}s", result.c_str());

    return result;
}

}  // namespace AppExecFwk
}  // namespace OHOS
