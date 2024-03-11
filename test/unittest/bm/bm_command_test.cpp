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

#include <gtest/gtest.h>

#define private public
#include "bundle_command.h"
#undef private
#include "bundle_installer_interface.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "mock_bundle_installer_host.h"
#include "mock_bundle_mgr_host.h"
#include "parameter.h"
#include "parameters.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
    const char* IS_ROOT_MODE_PARAM = "const.debuggable";
    const std::string IS_DEVELOPER_MODE_PARAM = "const.security.developermode.state";
    const int32_t ROOT_MODE = 1;
}

class BmCommandTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects();
    void SetMockObjects(BundleManagerShellCommand &cmd) const;

    sptr<IBundleMgr> mgrProxyPtr_;
    sptr<IBundleInstaller> installerProxyPtr_;
};

void BmCommandTest::SetUpTestCase()
{}

void BmCommandTest::TearDownTestCase()
{}

void BmCommandTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void BmCommandTest::TearDown()
{}

void BmCommandTest::MakeMockObjects()
{
    // mock a mgr host
    auto mgrHostPtr = sptr<IRemoteObject>(new (std::nothrow) MockBundleMgrHost());
    // mock a mgr proxy
    mgrProxyPtr_ = iface_cast<IBundleMgr>(mgrHostPtr);

    // mock a installer host
    auto installerHostPtr = sptr<IRemoteObject>(new (std::nothrow) MockBundleInstallerHost());
    // mock a installer proxy
    installerProxyPtr_ = iface_cast<IBundleInstaller>(installerHostPtr);
}

void BmCommandTest::SetMockObjects(BundleManagerShellCommand &cmd) const
{
    // set the mock mgr proxy
    cmd.bundleMgrProxy_ = mgrProxyPtr_;

    // set the mock installer proxy
    cmd.bundleInstallerProxy_ = installerProxyPtr_;
}

/**
 * @tc.number: Bm_Command_0001
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_0001, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);
    
    std::string message;
    message += HELP_MSG;
    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, ROOT_MODE);
    if (mode == ROOT_MODE) {
        message += ENABLE_DISABLE_HELP_MSG;
    }
    bool isDeveloperMode = system::GetBoolParameter(IS_DEVELOPER_MODE_PARAM, false);
    if (mode == ROOT_MODE || isDeveloperMode) {
        message += CLEAN_HELP_MSG;
    }

    EXPECT_EQ(cmd.ExecCommand(), message);
}

/**
 * @tc.number: Bm_Command_0002
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm xxx" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_0002, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    std::string message;
    message += HELP_MSG;
    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, ROOT_MODE);
    if (mode == ROOT_MODE) {
        message += ENABLE_DISABLE_HELP_MSG;
    }
    bool isDeveloperMode = system::GetBoolParameter(IS_DEVELOPER_MODE_PARAM, false);
    if (mode == ROOT_MODE || isDeveloperMode) {
        message += CLEAN_HELP_MSG;
    }

    EXPECT_EQ(cmd.ExecCommand(), cmd.GetCommandErrorMsg() + message);
}

/**
 * @tc.number: Bm_Command_0003
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm -xxx" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_0003, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("-xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    std::string message;
    message += HELP_MSG;
    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, ROOT_MODE);
    if (mode == ROOT_MODE) {
        message += ENABLE_DISABLE_HELP_MSG;
    }
    bool isDeveloperMode = system::GetBoolParameter(IS_DEVELOPER_MODE_PARAM, false);
    if (mode == ROOT_MODE || isDeveloperMode) {
        message += CLEAN_HELP_MSG;
    }

    EXPECT_EQ(cmd.ExecCommand(), cmd.GetCommandErrorMsg() + message);
}

/**
 * @tc.number: Bm_Command_0004
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm --xxx" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_0004, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("--xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    std::string message;
    message += HELP_MSG;
    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, ROOT_MODE);
    if (mode == ROOT_MODE) {
        message += ENABLE_DISABLE_HELP_MSG;
    }
    bool isDeveloperMode = system::GetBoolParameter(IS_DEVELOPER_MODE_PARAM, false);
    if (mode == ROOT_MODE || isDeveloperMode) {
        message += CLEAN_HELP_MSG;
    }

    EXPECT_EQ(cmd.ExecCommand(), cmd.GetCommandErrorMsg() + message);
}

/**
 * @tc.number: Bm_Command_0005
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm help" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_0005, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("help"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    std::string message;
    message += HELP_MSG;
    int32_t mode = GetIntParameter(IS_ROOT_MODE_PARAM, ROOT_MODE);
    if (mode == ROOT_MODE) {
        message += ENABLE_DISABLE_HELP_MSG;
    }
    bool isDeveloperMode = system::GetBoolParameter(IS_DEVELOPER_MODE_PARAM, false);
    if (mode == ROOT_MODE || isDeveloperMode) {
        message += CLEAN_HELP_MSG;
    }

    EXPECT_EQ(cmd.ExecCommand(), message);
}

/**
 * @tc.number: Bm_Command_Clean_0001
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0001, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0002
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean xx" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0002, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0003
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -n" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0003, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-n"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0004
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -n <bundle-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0004, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_DATA_OR_CACHE_OPTION + "\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0005
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -n <bundle-name> xxx" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0005, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_DATA_OR_CACHE_OPTION + "\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0006
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -n <bundle-name> -d" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0006, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-d"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_CLEAN_DATA_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Clean_0007
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -n <bundle-name> -c" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0007, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-c"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_CLEAN_CACHE_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Clean_0008
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -c" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0008, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-c"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0009
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -d" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0009, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-d"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0010
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -n <bundle-name> -d -u" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0010, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-d"),
        const_cast<char*>(" "),
        const_cast<char*>("-u"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0011
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -n <bundle-name> -d -u XXX" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0011, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-d"),
        const_cast<char*>(" "),
        const_cast<char*>("-u"),
        const_cast<char*>("XXX"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE);
}

/**
 * @tc.number: Bm_Command_Clean_0012
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -n <bundle-name> -d -u <user-id>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0012, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-d"),
        const_cast<char*>(" "),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_CLEAN_DATA_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Clean_0013
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -h" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0013, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-h"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "error: you must specify an option at least.\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0014
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -xxx" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0014, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-XXX"),
        const_cast<char*>(" "),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0015
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -n <bundle-name> -d -xxx <user-id>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0015, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-d"),
        const_cast<char*>(" "),
        const_cast<char*>("-XXX"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0016
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -xxx <bundle-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0016, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-xxx"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Clean_0017
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm clean -h" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Clean_0017, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("clean"),
        const_cast<char*>("-h"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_CLEAN);
}

/**
 * @tc.number: Bm_Command_Enable_0001
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0001, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_ENABLE);
}

/**
 * @tc.number: Bm_Command_Enable_0002
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -n <bundle-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0002, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-n"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_ENABLE);
}

/**
 * @tc.number: Bm_Command_Enable_0003
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -n <bundle-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0003, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_ENABLE_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Enable_0004
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -n <bundle-name> -a <ability-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0004, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-a"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_ENABLE);
}

/**
 * @tc.number: Bm_Command_Enable_0005
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -n <bundle-name> -a <ability-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0005, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-a"),
        const_cast<char*>(STRING_ABILITY_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_ENABLE_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Enable_0006
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -x" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0006, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-x"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_ENABLE);
}

/**
 * @tc.number: Bm_Command_Enable_0007
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -n <bundle-name> -u" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0007, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_ENABLE);
}

/**
 * @tc.number: Bm_Command_Enable_0008
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -n <bundle-name> -u XXX" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0008, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>("XXX"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE);
}

/**
 * @tc.number: Bm_Command_Enable_0009
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -n <bundle-name> -u <user-id>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0009, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_ENABLE_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Enable_0010
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -h" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0010, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-h"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_ENABLE);
}

/**
 * @tc.number: Bm_Command_Enable_0011
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm enable -n <bundle-name> -xxx <user-id>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Enable_0011, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("enable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-XXX"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_ENABLE);
}

/**
 * @tc.number: Bm_Command_Disable_0001
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0001, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_DISABLE);
}

/**
 * @tc.number: Bm_Command_Disable_0002
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -n <bundle-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0002, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-n"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_DISABLE);
}

/**
 * @tc.number: Bm_Command_Disable_0003
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -n <bundle-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0003, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_DISABLE_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Disable_0004
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -n <bundle-name> -a <ability-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0004, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-a"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_DISABLE);
}

/**
 * @tc.number: Bm_Command_Disable_0005
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -n <bundle-name> -a <ability-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0005, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-a"),
        const_cast<char*>(STRING_ABILITY_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_DISABLE_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Disable_0006
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -n <bundle-name> -u <user-id>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0006, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_DISABLE);
}

/**
 * @tc.number: Bm_Command_Disable_0007
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -n <bundle-name> -u <user-id>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0007, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>("100"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_DISABLE_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Disable_0008
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -x" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0008, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-x"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_DISABLE);
}

/**
 * @tc.number: Bm_Command_Disable_0009
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -h" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0009, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-h"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DISABLE);
}

/**
 * @tc.number: Bm_Command_Disable_0010
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -n <bundle-name> -u XXX" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0010, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-u"),
        const_cast<char*>("XXX"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE);
}

/**
 * @tc.number: Bm_Command_Disable_0011
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm disable -n <bundle-name> -xxx <ability-name>" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Disable_0011, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("disable"),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-XXX"),
        const_cast<char*>(STRING_ABILITY_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_DISABLE);
}

/**
 * @tc.number: Bm_Command_Get_0001
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0001, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_GET);
}

/**
 * @tc.number: Bm_Command_Get_0002
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get -u" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0002, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>("-u"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    std::string result = cmd.ExecCommand();
    auto pos = result.find(STRING_GET_UDID_OK);

    EXPECT_NE(pos, std::string::npos);
}

/**
 * @tc.number: Bm_Command_Get_0003
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get -x" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0003, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>("-x"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_INCORRECT_OPTION + "\n" + HELP_MSG_GET);
}

/**
 * @tc.number: Bm_Command_Get_0004
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get -u -x" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0004, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>("-u"),
        const_cast<char*>("-x"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_GET);
}

/**
 * @tc.number: Bm_Command_Get_0005
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get -u xxx" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0005, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>("-u"),
        const_cast<char*>("xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_GET);
}

/**
 * @tc.number: Bm_Command_Get_0006
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get --udid" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0006, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>("--udid"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    std::string result = cmd.ExecCommand();
    auto pos = result.find(STRING_GET_UDID_OK);

    EXPECT_NE(pos, std::string::npos);
}

/**
 * @tc.number: Bm_Command_Get_0007
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get --xxx" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0007, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>("--xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_INCORRECT_OPTION + "\n" + HELP_MSG_GET);
}

/**
 * @tc.number: Bm_Command_Get_0008
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get --udid -x" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0008, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>("--udid"),
        const_cast<char*>("-x"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_GET);
}

/**
 * @tc.number: Bm_Command_Get_0009
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get -u xxx" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0009, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>("--udid"),
        const_cast<char*>("xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_GET);
}

/**
 * @tc.number: Bm_Command_Get_0010
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm get -h" command.
 */
HWTEST_F(BmCommandTest, Bm_Command_Get_0010, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("get"),
        const_cast<char*>("-h"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_GET);
}

/**
 * @tc.number: GetBundlePath_0001
 * @tc.name: test GetBundlePath
 * @tc.desc: Verify the "GetBundlePath".
 */
HWTEST_F(BmCommandTest, GetBundlePath_0001, Function | MediumTest | Level1)
{
    // install a bundle
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>("-h"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);
    std::string param = "";
    std::vector<std::string> bundlePaths;
    auto res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    param = "-r";
    res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    param = "--replace";
    res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    param = "-p";
    res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    param = "--bundle-path";
    res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    param = "-u";
    res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    param = "--user-id";
    res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    param = "-w";
    res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    param = "--waitting-time";
    res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    param = "-x";
    res = cmd.GetBundlePath(param, bundlePaths);
    EXPECT_EQ(res, ERR_OK);
}
} // namespace OHOS