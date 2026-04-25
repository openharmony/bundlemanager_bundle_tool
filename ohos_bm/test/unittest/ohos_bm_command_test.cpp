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

#include <gtest/gtest.h>
#include <cstring>
#include "bundle_command.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class OhosBmCommandTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void OhosBmCommandTest::SetUpTestCase()
{}

void OhosBmCommandTest::TearDownTestCase()
{}

void OhosBmCommandTest::SetUp()
{}

void OhosBmCommandTest::TearDown()
{}

/**
 * @tc.name: HelpCommand_0100
 * @tc.desc: Test "ohos-bm help" command output.
 * @tc.type: FUNC
 */
HWTEST_F(OhosBmCommandTest, HelpCommand_0100, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>("ohos-bm"),
        const_cast<char *>("help"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    BundleManagerShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("install"), std::string::npos);
    EXPECT_NE(result.find("uninstall"), std::string::npos);
    EXPECT_NE(result.find("help"), std::string::npos);
}

/**
 * @tc.name: InstallCommand_0100
 * @tc.desc: Test "ohos-bm install" with no options.
 * @tc.type: FUNC
 */
HWTEST_F(OhosBmCommandTest, InstallCommand_0100, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>("ohos-bm"),
        const_cast<char *>("install"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    BundleManagerShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error"), std::string::npos);
}

/**
 * @tc.name: InstallCommand_0200
 * @tc.desc: Test "ohos-bm install -h" shows help message.
 * @tc.type: FUNC
 */
HWTEST_F(OhosBmCommandTest, InstallCommand_0200, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>("ohos-bm"),
        const_cast<char *>("install"),
        const_cast<char *>("-h"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    BundleManagerShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("bundle-path"), std::string::npos);
}

/**
 * @tc.name: UninstallCommand_0100
 * @tc.desc: Test "ohos-bm uninstall" with no options.
 * @tc.type: FUNC
 */
HWTEST_F(OhosBmCommandTest, UninstallCommand_0100, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>("ohos-bm"),
        const_cast<char *>("uninstall"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    BundleManagerShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("error"), std::string::npos);
}

/**
 * @tc.name: UninstallCommand_0200
 * @tc.desc: Test "ohos-bm uninstall -h" shows help message.
 * @tc.type: FUNC
 */
HWTEST_F(OhosBmCommandTest, UninstallCommand_0200, TestSize.Level0)
{
    char *argv[] = {
        const_cast<char *>("ohos-bm"),
        const_cast<char *>("uninstall"),
        const_cast<char *>("-h"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    BundleManagerShellCommand cmd(argc, argv);
    std::string result = cmd.ExecCommand();
    EXPECT_NE(result.find("bundle-name"), std::string::npos);
}
