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

#include <gtest/gtest.h>

#define private public
#include "bundle_command.h"
#undef private
#include "bundle_installer_interface.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "mock_bundle_installer_host.h"
#include "mock_bundle_mgr_host.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class BmCommandUninstallTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects();
    void SetMockObjects(BundleManagerShellCommand &cmd) const;

    std::string cmd_ = "uninstall";
    sptr<IBundleMgr> mgrProxyPtr_;
    sptr<IBundleInstaller> installerProxyPtr_;
};

void BmCommandUninstallTest::SetUpTestCase()
{}

void BmCommandUninstallTest::TearDownTestCase()
{}

void BmCommandUninstallTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void BmCommandUninstallTest::TearDown()
{}

void BmCommandUninstallTest::MakeMockObjects()
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

void BmCommandUninstallTest::SetMockObjects(BundleManagerShellCommand &cmd) const
{
    // set the mock mgr proxy
    cmd.bundleMgrProxy_ = mgrProxyPtr_;

    // set the mock installer proxy
    cmd.bundleInstallerProxy_ = installerProxyPtr_;
}

/**
 * @tc.number: Bm_Command_Uninstall_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_0100, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall xxx" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_0200, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -x" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_0300, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-x"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -xxx" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_0400, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall --x" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_0500, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--x"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall --xxx" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_0600, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall --h" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_0700, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-h"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall --help" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_0800, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--help"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_0900, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name>" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1000, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_UNINSTALL_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Uninstall_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -m" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1100, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -m <module-name>" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1200, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -m <module-name>" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1300, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_UNINSTALL_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Uninstall_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -u" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1400, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -u <user-id>" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1500, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
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

    EXPECT_EQ(cmd.ExecCommand(), STRING_UNINSTALL_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Uninstall_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -k" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1600, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-k"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_UNINSTALL_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Uninstall_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -k xxx" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1700, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-k"),
        const_cast<char*>("XXX"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_UNINSTALL_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Uninstall_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n xxx" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1800, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>("XXX"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_UNINSTALL_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Uninstall_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -xxx <bundle-name>" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_1900, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-XXX"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -u <user-id>" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_2000, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(ERR_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a correct value.\n");
}

/**
 * @tc.number: Bm_Command_Uninstall_2100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -k 1" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_2100, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-k"),
        const_cast<char*>("1"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_UNINSTALL_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Uninstall_2200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -cc <bundle-name>" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_2200, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-ccc"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_2300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -k 1 -v 1 -s" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_2300, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-k"),
        const_cast<char*>("1"),
        const_cast<char*>("-v"),
        const_cast<char*>("1"),
        const_cast<char*>("-s"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_UNINSTALL_BUNDLE_OK + "\n");
}

/**
 * @tc.number: Bm_Command_Uninstall_2400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -k xxxx" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_2400, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-k"),
        const_cast<char*>("xxxx"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_2500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -v 1" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_2500, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-v"),
        const_cast<char*>("1"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_2600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -s" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_2600, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-s"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" + HELP_MSG_UNINSTALL);
}

/**
 * @tc.number: Bm_Command_Uninstall_2700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm uninstall -n <bundle-name> -k" command.
 */
HWTEST_F(BmCommandUninstallTest, Bm_Command_Uninstall_2700, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-k"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_UNINSTALL_BUNDLE_OK + "\n");
}
} // OHOS