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
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class BmCommandOverlayTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects();
    void SetMockObjects(BundleManagerShellCommand &cmd) const;

    std::string overlay_ = "dump-overlay";
    std::string tOverlay_ = "dump-target-overlay";
    sptr<IBundleMgr> mgrProxyPtr_;
    sptr<IBundleInstaller> installerProxyPtr_;
};

void BmCommandOverlayTest::SetUpTestCase()
{}

void BmCommandOverlayTest::TearDownTestCase()
{}

void BmCommandOverlayTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void BmCommandOverlayTest::TearDown()
{}

void BmCommandOverlayTest::MakeMockObjects()
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

void BmCommandOverlayTest::SetMockObjects(BundleManagerShellCommand &cmd) const
{
    // set the mock mgr proxy
    cmd.bundleMgrProxy_ = mgrProxyPtr_;

    // set the mock installer proxy
    cmd.bundleInstallerProxy_ = installerProxyPtr_;
}

/**
 * @tc.number: Bm_Command_Overlay_0001
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0001, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_OVERLAY);
}

/**
 * @tc.number: Bm_Command_Overlay_0002
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -xxx" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0002, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_OVERLAY);
}

/**
 * @tc.number: Bm_Command_Overlay_0003
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -b" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0003, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_OVERLAY);
}

/**
 * @tc.number: Bm_Command_Overlay_0004
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -m" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0004, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_OVERLAY);
}

/**
 * @tc.number: Bm_Command_Overlay_0005
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -t" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0005, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-t"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_OVERLAY);
}

/**
 * @tc.number: Bm_Command_Overlay_0006
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -u" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0006, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_OVERLAY);
}

/**
 * @tc.number: Bm_Command_Overlay_0007
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -h" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0007, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-h"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_OVERLAY);
}

/**
 * @tc.number: Bm_Command_Overlay_0008
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -b <bundle-name>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0008, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Overlay_0009
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -m <module-name>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0009, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Overlay_0010
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -t <target-module-name>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0010, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-t"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Overlay_0011
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -u <user-id>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0011, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>("N"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE);
}

/**
 * @tc.number: Bm_Command_Overlay_0012
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -u <user-id>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0012, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Overlay_0013
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -u <user-id>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0013, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(ERR_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE);
}

/**
 * @tc.number: Bm_Command_Overlay_0014
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -b <bundle-name> -m -t" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0014, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Overlay_0015
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -b <bundle-name> -m <module-name> -t" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0015, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Overlay_0016
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -b <bundle-name> -m -t <target-module-name>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0016, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-t"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Overlay_0017
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -b <bundle-name>
 *     -m <module-name> -t <target-module-name>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Overlay_0017, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(overlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>("-t"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_OVERLAY);
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0001
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-target-overlay" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0001, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_OVERLAY_TARGET);
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0002
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-target-overlay -b" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0002, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_OVERLAY_TARGET);
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0003
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-target-overlay -xxx" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0003, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-xxx"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_OVERLAY_TARGET);
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0004
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-target-overlay -m" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0004, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_OVERLAY_TARGET);
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0005
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-target-overlay -u" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0005, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_OVERLAY_TARGET);
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0006
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-target-overlay -h" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0006, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-h"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_OVERLAY_TARGET);
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0007
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-target-overlay -b <bundle-name>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0007, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_TARGET_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0008
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -m <module-name>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0008, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_TARGET_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0009
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -u <user-id>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0009, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>("N"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE);
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0010
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -u <user-id>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0010, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_TARGET_OVERLAY_NG + "\n");
}


/**
 * @tc.number: Bm_Command_Target_Overlay_0011
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -u <user-id>" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0011, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(ERR_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE);
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0012
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -b <bundle-name> -m <module-name> -t" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0012, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_TARGET_OVERLAY_NG + "\n");
}

/**
 * @tc.number: Bm_Command_Target_Overlay_0013
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-overlay -b <bundle-name> -m" command.
 */
HWTEST_F(BmCommandOverlayTest, Bm_Command_Target_Overlay_0013, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(tOverlay_.c_str()),
        const_cast<char*>("-b"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-u"),
        const_cast<char*>(DEFAULT_USER_ID.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), STRING_DUMP_TARGET_OVERLAY_NG + "\n");
}
} // namespace OHOS