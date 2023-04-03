/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "tool_system_test.h"

#include <gtest/gtest.h>
#include <thread>

using namespace OHOS::AppExecFwk;

namespace {
const std::string STRING_INSTALL_BUNDLE_OK = "install bundle successfully.";
const std::string STRING_UNINSTALL_BUNDLE_OK = "uninstall bundle successfully.";
}  // namespace

std::string ToolSystemTest::ExecuteCommand(const std::string& command)
{
    std::string result = "";
    FILE* file = popen(command.c_str(), "r");

    // wait for services
    std::this_thread::sleep_for(std::chrono::seconds(TIME_DELAY_FOR_SERVICES));

    if (file != nullptr) {
        char commandResult[1024] = { 0 };
        while ((fgets(commandResult, sizeof(commandResult), file)) != nullptr) {
            result.append(commandResult);
        }
        pclose(file);
        file = nullptr;
    }

    return result;
}

void ToolSystemTest::InstallBundle(const std::string& bundlePath, const bool checkResult)
{
    // install a bundle
    std::string command = "bm install -p " + bundlePath;
    std::string commandResult = ExecuteCommand(command);

    if (checkResult) {
        EXPECT_PRED2(ToolSystemTest::IsSubSequence, commandResult, STRING_INSTALL_BUNDLE_OK + "\n");
    }
}

void ToolSystemTest::UninstallBundle(const std::string& bundleName, const bool checkResult)
{
    // uninstall a bundle
    std::string command = "bm uninstall -n " + bundleName;
    std::string commandResult = ExecuteCommand(command);

    if (checkResult) {
        EXPECT_PRED2(ToolSystemTest::IsSubSequence, commandResult, STRING_UNINSTALL_BUNDLE_OK + "\n");
    }
}

bool ToolSystemTest::IsSubSequence(const std::string& str, const std::string& subStr)
{
    return str.find(subStr) != std::string::npos;
}
