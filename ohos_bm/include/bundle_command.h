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
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string TOOL_NAME = "ohos-bm";

const std::string HELP_MSG = "usage: ohos-bm <command> <options>\n"
                             "These are common ohos-bm commands list:\n"
                             "  help              list available commands\n"
                             "  install           install a bundle with options\n"
                             "  uninstall         uninstall a bundle with options\n"
                             "  dump              dump the bundle info\n"
                             "  dump-dependencies dump dependencies by given bundle name and module name\n"
                             "  dump-shared       dump inter-application shared library information by bundle name\n"
                             "  clean             clean the bundle data\n";

const std::string HELP_MSG_INSTALL =
    "usage: ohos-bm install <options>\n"
    "options list:\n"
    "  -h, --help                                                     list available commands\n"
    "  -p, --bundle-path <file-path>                                  install a hap or hsp or app by a specified path\n"
    "  -p, --bundle-path <file-path> <file-path> ...                  install one bundle by some hap or hsp paths\n"
    "  -p, --bundle-path <bundle-direction>                           install one bundle by a direction,\n"
    "                                                                    under which are some hap or hsp\n"
    "                                                                    or one app files\n"
    "  -r -p <bundle-file-path>                                       replace an existing bundle\n"
    "  -r --bundle-path <bundle-file-path>                            replace an existing bundle\n"
    "  -s, --shared-bundle-dir-path <shared-bundle-dir-path>          install inter-application hsp files\n"
    "  -u, --user-id <user-id>                                        specify a user id,\n"
    "                                                                   only supports current user or userId is 0\n"
    "  -w, --waitting-time <waitting-time>                            specify waitting time for installation,\n"
    "                                                                    the minimum waitting time is 180s,\n"
    "                                                                    the maximum waitting time is 600s\n"
    "  -d, --downgrade                                                install allow downgrade\n"
    "  -g, --grant-permission                                         grant permissions for installation\n";

const std::string HELP_MSG_UNINSTALL =
    "usage: ohos-bm uninstall <options>\n"
    "options list:\n"
    "  -h, --help                           list available commands\n"
    "  -n, --bundle-name <bundle-name>      uninstall a bundle by bundle name\n"
    "  -m, --module-name <module-name>      uninstall a module by module name\n"
    "  -u, --user-id <user-id>              specify a user id,only supports current user or userId is 0\n"
    "  -k, --keep-data                      keep the user data after uninstall\n"
    "  -s, --shared                         uninstall inter-application shared library\n"
    "  -v, --version                        uninstall a inter-application shared library by versionCode\n";

const std::string HELP_MSG_DUMP =
    "usage: ohos-bm dump <options>\n"
    "options list:\n"
    "  -h, --help                           list available commands\n"
    "  -a, --all                            list all bundles in system\n"
    "  -g, --debug-bundle                   list debug bundles in system\n"
    "  -n, --bundle-name <bundle-name>      list the bundle info by a bundle name\n"
    "  -s, --shortcut-info                  list the shortcut info\n"
    "  -d, --device-id <device-id>          specify a device id\n"
    "  -u, --user-id <user-id>              specify a user id,only supports current user or userId is 0\n"
    "  -l, --label                          list the label info\n";

const std::string HELP_MSG_CLEAN =
    "usage: ohos-bm clean <options>\n"
    "options list:\n"
    "  -h, --help                                      list available commands\n"
    "  -n, --bundle-name  <bundle-name>                bundle name\n"
    "  -c, --cache                                     clean bundle cache files by bundle name\n"
    "  -d, --data                                      clean bundle data files by bundle name\n"
    "  -u, --user-id <user-id>                         specify a user id,only supports current user or userId is 0\n"
    "  -i, --app-index <app-index>                     specify a app index\n";

const std::string HELP_MSG_DUMP_SHARED =
    "usage: ohos-bm dump-shared <options>\n"
    "eg:ohos-bm dump-shared -n <bundle-name> \n"
    "options list:\n"
    "  -h, --help                             list available commands\n"
    "  -a, --all                              list all inter-application shared library name in system\n"
    "  -n, --bundle-name  <bundle-name>       dump inter-application shared library information by bundleName\n";

const std::string HELP_MSG_DUMP_SHARED_DEPENDENCIES =
    "usage: ohos-bm dump-dependencies <options>\n"
    "eg:ohos-bm dump-dependencies -n <bundle-name> -m <module-name> \n"
    "options list:\n"
    "  -h, --help                             list available commands\n"
    "  -n, --bundle-name  <bundle-name>       dump dependencies by bundleName and moduleName\n"
    "  -m, --module-name  <module-name>       dump dependencies by bundleName and moduleName\n";

const std::string STRING_INCORRECT_OPTION = "error: incorrect option";
const std::string HELP_MSG_NO_BUNDLE_PATH_OPTION =
    "error: you must specify a bundle path with '-p' or '--bundle-path'.";

const std::string HELP_MSG_NO_BUNDLE_NAME_OPTION =
    "error: you must specify a bundle name with '-n' or '--bundle-name'.";

const std::string STRING_INSTALL_BUNDLE_OK = "install bundle successfully.";
const std::string STRING_INSTALL_BUNDLE_NG = "error: failed to install bundle.";

const std::string STRING_UNINSTALL_BUNDLE_OK = "uninstall bundle successfully.";
const std::string STRING_UNINSTALL_BUNDLE_NG = "error: failed to uninstall bundle.";

const std::string HELP_MSG_NO_DATA_OR_CACHE_OPTION =
    "error: you must specify '-c' or '-d' for 'ohos-bm clean' option.";
const std::string STRING_CLEAN_CACHE_BUNDLE_OK = "clean bundle cache files successfully.";
const std::string STRING_CLEAN_CACHE_BUNDLE_NG = "error: failed to clean bundle cache files.";

const std::string STRING_CLEAN_DATA_BUNDLE_OK = "clean bundle data files successfully.";
const std::string STRING_CLEAN_DATA_BUNDLE_NG = "error: failed to clean bundle data files.";

const std::string STRING_REQUIRE_CORRECT_VALUE = "error: option requires a correct value.\n";

const std::string HELP_MSG_DUMP_FAILED = "error: failed to get information and the parameters may be wrong.";

const std::string HELP_MSG_NO_REMOVABLE_OPTION =
    "error: you must specify a bundle name with '-n' or '--bundle-name' \n"
    "and a module name with '-m' or '--module-name' \n";

const std::string BUNDLE_NAME_EMPTY = "";
const std::string SHARED_BUNDLE_INFO = "sharedBundleInfo";
const std::string DEPENDENCIES = "dependencies";

const int32_t MAX_WAITING_TIME = 3000;
const int32_t INITIAL_SANDBOX_APP_INDEX = 1000;

const std::string WARNING_USER =
    "Warning: The current user is %. If you want to set the userId as $, please switch to $.\n";
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

    ErrCode RunAsHelpCommand();
    ErrCode RunAsInstallCommand();
    ErrCode RunAsUninstallCommand();
    ErrCode RunAsDumpCommand();
    ErrCode RunAsDumpSharedDependenciesCommand();
    ErrCode RunAsDumpSharedCommand();
    ErrCode RunAsCleanCommand();

    int32_t InstallOperation(const std::vector<std::string> &bundlePaths, InstallParam &installParam,
        int32_t waittingTime, std::string &resultMsg) const;
    int32_t UninstallOperation(const std::string &bundleName, const std::string &moduleName,
                               InstallParam &installParam) const;
    int32_t UninstallSharedOperation(const UninstallParam &uninstallParam) const;
    bool IsInstallOption(int index) const;
    void GetAbsPaths(const std::vector<std::string> &paths, std::vector<std::string> &absPaths) const;

    ErrCode GetBundlePath(const std::string& param, std::vector<std::string>& bundlePaths) const;
    std::string GetWaringString(int32_t currentUserId, int32_t specifedUserId) const;

    std::string DumpBundleList(int32_t userId) const;
    std::string DumpDebugBundleList(int32_t userId) const;
    std::string DumpBundleInfo(const std::string &bundleName, int32_t userId) const;
    std::string DumpShortcutInfos(const std::string &bundleName, int32_t userId) const;
    std::string DumpDistributedBundleInfo(const std::string &deviceId, const std::string &bundleName);
    std::string DumpSharedDependencies(const std::string &bundleName, const std::string &moduleName) const;
    std::string DumpShared(const std::string &bundleName) const;
    std::string DumpSharedAll() const;
    std::string DumpAllLabel(int32_t userId) const;
    std::string DumpBundleLabel(const std::string &bundleName, int32_t userId) const;

    bool CleanBundleCacheFilesOperation(const std::string &bundleName, int32_t userId, int32_t appIndex = 0) const;
    bool CleanBundleDataFilesOperation(const std::string &bundleName, int32_t userId, int32_t appIndex = 0) const;

    ErrCode ParseSharedDependenciesCommand(int32_t option, std::string &bundleName, std::string &moduleName);
    ErrCode ParseSharedCommand(int32_t option, std::string &bundleName, bool &dumpSharedAll);

    // JSON output helper methods
    std::string CreateSuccessResult(const std::string &message, const std::string &data = "") const;
    std::string CreateErrorResult(const std::string &code, const std::string &message,
        const std::string &cause = "", const std::string &suggestion = "") const;
    std::string GetMessageFromCode(int32_t code) const;

    sptr<IBundleMgr> bundleMgrProxy_;
    sptr<IBundleInstaller> bundleInstallerProxy_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_BUNDLEMANAGER_BUNDLE_TOOL_OHOS_BM_INCLUDE_BUNDLE_COMMAND_H
