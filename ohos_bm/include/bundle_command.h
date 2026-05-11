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

#ifndef FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_BUNDLE_COMMAND_H
#define FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_BUNDLE_COMMAND_H

#include "shell_command.h"
#include "bundle_mgr_interface.h"
#include "bundle_installer_interface.h"
#include "app_control_interface.h"
#include "disposed_rule.h"
#include "status_receiver_interface.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string TOOL_NAME = "ohos-bm";

const std::string HELP_MSG = "usage: ohos-bm <command> <options>\n"
                             "These are common ohos-bm commands list:\n"
                             "  --help            list available commands\n"
                             "  uninstall         uninstall a bundle with options\n"
                             "  dump              dump the bundle info\n"
                             "  dump-dependencies dump dependencies by given bundle name and module name\n"
                             "  dump-shared       dump inter-application shared library information by bundle name\n"
                             "  clean             clean the bundle data\n"
                             "  set-disposed-rule set disposed rule for clone app\n"
                             "  delete-disposed-rule delete disposed rule for clone app\n";

const std::string HELP_MSG_UNINSTALL =
    "usage: ohos-bm uninstall <options>\n"
    "options list:\n"
    "  --help                           list available commands\n"
    "  --bundleName <bundle-name>      uninstall a bundle by bundle name\n"
    "  --keepData                      keep the user data after uninstall\n"
    "  --shared                         uninstall inter-application shared library\n"
    "  --version                        uninstall a inter-application shared library by versionCode\n";

const std::string HELP_MSG_DUMP =
    "usage: ohos-bm dump <options>\n"
    "options list:\n"
    "  --help                           list available commands\n"
    "  --all                            list all bundles in system\n"
    "  --debugBundle                   list debug bundles in system\n"
    "  --bundleName <bundle-name>      list the bundle info by a bundle name\n"
    "  --shortcutInfo                  list the shortcut info\n"
    "  --deviceId <device-id>          specify a device id\n"
    "  --label                          list the label info\n";

const std::string HELP_MSG_CLEAN =
    "usage: ohos-bm clean <options>\n"
    "options list:\n"
    "  --help                                      list available commands\n"
    "  --bundleName  <bundle-name>                bundle name\n"
    "  --cache                                     clean bundle cache files by bundle name\n"
    "  --data                                      clean bundle data files by bundle name\n"
    "  --appIndex <app-index>                     specify a app index\n";

const std::string HELP_MSG_DUMP_SHARED =
    "usage: ohos-bm dump-shared <options>\n"
    "eg:ohos-bm dump-shared --bundleName <bundle-name> \n"
    "options list:\n"
    "  --help                             list available commands\n"
    "  --all                              list all inter-application shared library name in system\n"
    "  --bundleName  <bundle-name>       dump inter-application shared library information by bundleName\n";

const std::string HELP_MSG_DUMP_SHARED_DEPENDENCIES =
    "usage: ohos-bm dump-dependencies <options>\n"
    "eg:ohos-bm dump-dependencies --bundleName <bundle-name> --moduleName <module-name> \n"
    "options list:\n"
    "  --help                             list available commands\n"
    "  --bundleName  <bundle-name>       dump dependencies by bundleName and moduleName\n"
    "  --moduleName  <module-name>       dump dependencies by bundleName and moduleName\n";

const std::string HELP_MSG_NO_OPTION =
    "error: no option specified. Use --help for help.";

const std::string HELP_MSG_NO_BUNDLE_NAME_OPTION =
    "error: you must specify a bundle name with --bundleName.";

const std::string STRING_UNINSTALL_BUNDLE_OK = "uninstall bundle successfully.";
const std::string STRING_UNINSTALL_BUNDLE_NG = "error: failed to uninstall bundle.";

const std::string HELP_MSG_NO_DATA_OR_CACHE_OPTION =
    "error: you must specify --cache or --data for 'ohos-bm clean' option.";
const std::string STRING_CLEAN_CACHE_BUNDLE_OK = "clean bundle cache files successfully.";
const std::string STRING_CLEAN_CACHE_BUNDLE_NG = "error: failed to clean bundle cache files.";

const std::string STRING_CLEAN_DATA_BUNDLE_OK = "clean bundle data files successfully.";
const std::string STRING_CLEAN_DATA_BUNDLE_NG = "error: failed to clean bundle data files.";

const std::string STRING_REQUIRE_CORRECT_VALUE = "error: option requires a correct value.\n";

const std::string HELP_MSG_DUMP_FAILED = "error: failed to get information and the parameters may be wrong.";

const std::string HELP_MSG_NO_REMOVABLE_OPTION =
    "error: you must specify a bundle name with --bundleName \n"
    "and a module name with --moduleName \n";

const std::string BUNDLE_NAME_EMPTY = "";
const std::string SHARED_BUNDLE_INFO = "sharedBundleInfo";
const std::string DEPENDENCIES = "dependencies";

const int32_t MAX_WAITING_TIME = 3000;
const int32_t INITIAL_SANDBOX_APP_INDEX = 1000;

const std::string HELP_MSG_SET_DISPOSED_RULE =
    "usage: ohos-bm set-disposed-rule <options> "
    "options list: "
    "--help: list available commands. "
    "--appId <app-id>: application appId or appIdentifier (required). "
    "--appIndex <app-index>: clone app index, a positive integer. "
    "--priority <priority>: priority of the disposed rule, a non-negative integer (required). "
    "--componentType <type>: component type to control (required): 1=UI_ABILITY, 2=UI_EXTENSION. "
    "--disposedType <type>: disposal type (required): 1=BLOCK_APPLICATION, 2=BLOCK_ABILITY, 3=NON_BLOCK. "
    "--controlType <type>: control type for elementList (required): 1=ALLOWED_LIST, 2=DISALLOWED_LIST. "
    "--elements <element-uri>: element to control, format: /bundleName/moduleName/abilityName, "
    "multiple elements can be added by repeating this option. "
    "--wantBundleName <name>: bundleName of the Want for redirection (required). "
    "--wantModuleName <name>: moduleName of the Want for redirection. "
    "--wantAbilityName <name>: abilityName of the Want for redirection (required). "
    "--wantParamsStrings \"<key> <value>\": Want string parameter (repeatable). "
    "--wantParamsInts \"<key> <value>\": Want int parameter (repeatable). "
    "--wantParamsBools \"<key> <true/false>\": Want bool parameter (repeatable).";

const std::string HELP_MSG_DELETE_DISPOSED_RULE =
    "usage: ohos-bm delete-disposed-rule <options> "
    "options list: "
    "--help: list available commands. "
    "--appId <app-id>: application appId or appIdentifier (required). "
    "--appIndex <app-index>: clone app index, a positive integer.";

const std::string STRING_SET_DISPOSED_RULE_OK = "set disposed rule successfully.";
const std::string STRING_SET_DISPOSED_RULE_NG = "error: failed to set disposed rule.";
const std::string STRING_DELETE_DISPOSED_RULE_OK = "delete disposed rule successfully.";
const std::string STRING_DELETE_DISPOSED_RULE_NG = "error: failed to delete disposed rule.";
const std::string STRING_USER_ID_INVALID = "error: userId is invalid, cannot call this command.";

constexpr int32_t OPTION_APP_ID = 1000;
constexpr int32_t OPTION_APP_INDEX = 1001;
constexpr int32_t OPTION_PRIORITY = 1002;
constexpr int32_t OPTION_COMPONENT_TYPE = 1003;
constexpr int32_t OPTION_DISPOSED_TYPE = 1004;
constexpr int32_t OPTION_CONTROL_TYPE = 1005;
constexpr int32_t OPTION_ELEMENT = 1006;
constexpr int32_t OPTION_WANT_BUNDLE_NAME = 1007;
constexpr int32_t OPTION_WANT_MODULE_NAME = 1008;
constexpr int32_t OPTION_WANT_ABILITY_NAME = 1009;
constexpr int32_t OPTION_WANT_PS = 1010;
constexpr int32_t OPTION_WANT_PI = 1011;
constexpr int32_t OPTION_WANT_PB = 1012;
} // namespace

class BundleManagerShellCommand : public ShellCommand {
public:
    BundleManagerShellCommand(int argc, char *argv[]);
    ~BundleManagerShellCommand() override
    {}

private:
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode Init() override;
    ErrCode InitInstaller();
    ErrCode InitAppControlProxy();

    ErrCode RunAsHelpCommand();
    ErrCode RunAsUninstallCommand();
    ErrCode RunAsDumpCommand();
    ErrCode RunAsDumpSharedDependenciesCommand();
    ErrCode RunAsDumpSharedCommand();
    ErrCode RunAsCleanCommand();
    ErrCode RunAsSetDisposedRuleCommand();
    ErrCode RunAsDeleteDisposedRuleCommand();

    int32_t UninstallOperation(const std::string &bundleName,
                               InstallParam &installParam) const;
    int32_t UninstallSharedOperation(const UninstallParam &uninstallParam) const;

    std::string DumpBundleList(int32_t userId) const;
    std::string DumpDebugBundleList(int32_t userId) const;
    std::string DumpBundleInfo(const std::string &bundleName, int32_t userId) const;
    std::string DumpShortcutInfos(const std::string &bundleName, int32_t userId) const;
    std::string DumpDistributedBundleInfo(const std::string &deviceId, const std::string &bundleName);
    std::string DumpAllLabel(int32_t userId) const;
    std::string DumpBundleLabel(const std::string &bundleName, int32_t userId) const;
    
    ErrCode ParseParamInteger(std::map<std::string, int>& pi);
    ErrCode ParseParamBool(std::map<std::string, bool>& pb);
    ErrCode ParseParamString(std::map<std::string, std::string>& ps);

    bool CleanBundleCacheFilesOperation(const std::string &bundleName, int32_t userId, int32_t appIndex = 0) const;
    bool CleanBundleDataFilesOperation(const std::string &bundleName, int32_t userId, int32_t appIndex = 0) const;

    ErrCode ParseSharedDependenciesCommand(int32_t option, std::string &bundleName, std::string &moduleName);
    ErrCode ParseSharedCommand(int32_t option, std::string &bundleName, bool &dumpSharedAll);

    // JSON output helper methods
    std::string CreateSuccessResult(const std::string &data = "") const;
    std::string CreateErrorResult(int32_t code, const std::string &message,
        const std::string &suggestion = "") const;
    std::string CreateErrorResult(const std::string &errCode, const std::string &message,
        const std::string &suggestion = "") const;

    sptr<IBundleMgr> bundleMgrProxy_;
    sptr<IBundleInstaller> bundleInstallerProxy_;
    sptr<IAppControlMgr> appControlProxy_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_BUNDLE_COMMAND_H
