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
#include "bundle_constants.h"
#include "bundle_installer_interface.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "mock_bundle_installer_host.h"
#include "mock_bundle_mgr_host.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class BmCommandDumpDependenciesTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects();
    void SetMockObjects(BundleManagerShellCommand &cmd) const;

    std::string cmd_ = "dump-dependencies";
    sptr<IBundleMgr> mgrProxyPtr_;
    sptr<IBundleInstaller> installerProxyPtr_;
};

void BmCommandDumpDependenciesTest::SetUpTestCase()
{}

void BmCommandDumpDependenciesTest::TearDownTestCase()
{}

void BmCommandDumpDependenciesTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void BmCommandDumpDependenciesTest::TearDown()
{}

void BmCommandDumpDependenciesTest::MakeMockObjects()
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

void BmCommandDumpDependenciesTest::SetMockObjects(BundleManagerShellCommand &cmd) const
{
    // set the mock mgr proxy
    cmd.bundleMgrProxy_ = mgrProxyPtr_;

    // set the mock installer proxy
    cmd.bundleInstallerProxy_ = installerProxyPtr_;
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_0100, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies xxx" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_0200, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -x" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_0300, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -xxx" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_0400, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies --x" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_0500, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies --xxx" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_0600, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -h" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_0700, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies --help" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_0800, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -n" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_0900, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -n <bundle-name>" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_1000, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_REMOVABLE_OPTION + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -m" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_1100, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-m"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleManagerShellCommand cmd(argc, argv);
    // set the mock objects
    SetMockObjects(cmd);
    EXPECT_EQ(cmd.ExecCommand(), STRING_REQUIRE_CORRECT_VALUE + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -m <module-name>" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_1200, Function | MediumTest | Level1)
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
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_REMOVABLE_OPTION + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -n <bundle-name> -m <module-name>" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_1300, Function | MediumTest | Level1)
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

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_DUMP_FAILED + "\n");
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -n <bundle-name> -XXX <module-name>" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_1400, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-n"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>("-XXX"),
        const_cast<char*>(STRING_MODULE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}

/**
 * @tc.number: Bm_Command_Dump_DumpDependencies_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "bm dump-dependencies -xxx <bundle-name>" command.
 */
HWTEST_F(BmCommandDumpDependenciesTest, Bm_Command_Dump_DumpDependencies_1500, Function | MediumTest | Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-xxx"),
        const_cast<char*>(STRING_BUNDLE_NAME.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    // set the mock objects
    SetMockObjects(cmd);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_DUMP_SHARED_DEPENDENCIES);
}
} // namespace OHOS