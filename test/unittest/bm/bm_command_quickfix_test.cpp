/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#define protected public
#include "bundle_command.h"
#include "quick_fix_command.h"
#undef protected

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
const int QUICK_FIX_COPY_FILES_FAILED = 4;
const int QUICK_FIX_GET_BUNDLE_INFO_FAILED = 8;
const int QUICK_FIX_INVALID_VALUE = 22;
const int QUICK_FIX_OK = 0;

namespace OHOS {
namespace AppExecFwk {
class BmCommandQuickFixTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::string cmd_ = "quickfix";
};

void BmCommandQuickFixTest::SetUpTestCase()
{}

void BmCommandQuickFixTest::TearDownTestCase()
{}

void BmCommandQuickFixTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void BmCommandQuickFixTest::TearDown()
{}

/**
 * @tc.name: Bm_Command_QuickFix_0100
 * @tc.desc: "bm quickfix" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_0100, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);
    
    EXPECT_EQ(cmd.ExecCommand(), "error: parameter is not enough.\n" + HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_0200
 * @tc.desc: "bm quickfix --invalid" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_0200, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--invalid"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Help_0100
 * @tc.desc: "bm quickfix -h" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Help_0100, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-h"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Help_0200
 * @tc.desc: "bm quickfix --help" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Help_0200, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--help"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Apply_0100
 * @tc.desc: "bm quickfix -a" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Apply_0100, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-a"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "error: option [--apply] is incorrect.\n" + HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Apply_0200
 * @tc.desc: "bm quickfix --apply" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Apply_0200, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--apply"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "error: option [--apply] is incorrect.\n" + HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Apply_0300
 * @tc.desc: "bm quickfix --apply --invalidKey" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Apply_0300, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--apply"),
        const_cast<char*>("--invalidKey"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "error: option [--apply] is incorrect.\n" + HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Apply_0400
 * @tc.desc: "bm quickfix --apply --invalidKey invalidValue" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Apply_0400, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--apply"),
        const_cast<char*>("--invalidKey"),
        const_cast<char*>("invalidValue"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "error: parameter is not enough.\n" + HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Apply_0500
 * @tc.desc: "bm quickfix --apply -f <file-path>" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Apply_0500, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--apply"),
        const_cast<char*>("-f"),
        const_cast<char*>("/data/storage/el1/aa.hqf"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "apply quickfix failed with errno: 4.\n");
}

/**
 * @tc.name: Bm_Command_QuickFix_Apply_0600
 * @tc.desc: "bm quickfix --apply --file-path <file-path> <file-path>" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Apply_0600, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--apply"),
        const_cast<char*>("--file-path"),
        const_cast<char*>("/data/storage/el1/aa.hqf"),
        const_cast<char*>("/data/storage/el2/bb.hqf"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "apply quickfix failed with errno: 4.\n");
}

/**
 * @tc.name: Bm_Command_QuickFix_Apply_0700
 * @tc.desc: "bm quickfix --apply --file-path <bundle-direction>" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Apply_0700, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--apply"),
        const_cast<char*>("--file-path"),
        const_cast<char*>("/data/storage/el1"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "apply quickfix failed with errno: 4.\n");
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_0100
 * @tc.desc: "bm quickfix -q" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_0100, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-q"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "error: option [--query] is incorrect.\n" + HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_0200
 * @tc.desc: "bm quickfix --query" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_0200, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--query"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "error: option [--query] is incorrect.\n" + HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_0300
 * @tc.desc: "bm quickfix -q --invalidKey" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_0300, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("-q"),
        const_cast<char*>("--invalidKey"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "error: option [--query] is incorrect.\n" + HELP_MSG_QUICK_FIX);
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_0400
 * @tc.desc: "bm quickfix --query --invalidKey invalidValue" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_0400, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--query"),
        const_cast<char*>("--invalidKey"),
        const_cast<char*>("invalidValue"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "bundle name is empty.\n");
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_0500
 * @tc.desc: "bm quickfix --query -b <bundle-name>" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_0500, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--query"),
        const_cast<char*>("-b"),
        const_cast<char*>("bundleName1"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "Get quick fix info failed with errno 8.\n");
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_0600
 * @tc.desc: "bm quickfix --query --bundle-name <bundle-name>" test.
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_0600, TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>(TOOL_NAME.c_str()),
        const_cast<char*>(cmd_.c_str()),
        const_cast<char*>("--query"),
        const_cast<char*>("--bundle-name"),
        const_cast<char*>("bundleName1"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    BundleManagerShellCommand cmd(argc, argv);

    EXPECT_EQ(cmd.ExecCommand(), "Get quick fix info failed with errno 8.\n");
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_0700
 * @tc.desc: Test quickFixFiles is empty
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_0700, TestSize.Level1)
{
    QuickFixCommand command;
    std::vector<std::string> quickFixFiles;
    std::string resultInfo;
    auto ret = command.ApplyQuickFix(quickFixFiles, resultInfo);
    EXPECT_EQ(resultInfo, "quick fix file is empty.\n");
    EXPECT_EQ(ret, QUICK_FIX_INVALID_VALUE);
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_0800
 * @tc.desc: Test dir not have hqf file
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_0800, TestSize.Level1)
{
    QuickFixCommand command;
    std::vector<std::string> quickFixFiles = { "/data/null" };
    std::string resultInfo;
    auto ret = command.ApplyQuickFix(quickFixFiles, resultInfo);
    EXPECT_EQ(resultInfo, "apply quickfix failed with errno: 4.\n");
    EXPECT_EQ(ret, QUICK_FIX_COPY_FILES_FAILED);
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_0900
 * @tc.desc: Test empty bundleName
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_0900, TestSize.Level1)
{
    QuickFixCommand command;
    std::string bundleName;
    std::string resultInfo;
    auto ret = command.GetApplyedQuickFixInfo(bundleName, resultInfo);
    EXPECT_EQ(resultInfo, "bundle name is empty.\n");
    EXPECT_EQ(ret, QUICK_FIX_INVALID_VALUE);
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_1000
 * @tc.desc: Test right bundleName
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_1000, TestSize.Level1)
{
    QuickFixCommand command;
    std::string bundleName = "com.ohos.launcher";
    std::string resultInfo;
    auto ret = command.GetApplyedQuickFixInfo(bundleName, resultInfo);
    EXPECT_EQ(ret, QUICK_FIX_OK);
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_1100
 * @tc.desc: Test wrong bundleName
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_1100, TestSize.Level1)
{
    QuickFixCommand command;
    std::string bundleName = "wrong";
    std::string resultInfo;
    auto ret = command.GetApplyedQuickFixInfo(bundleName, resultInfo);
    EXPECT_EQ(ret, QUICK_FIX_GET_BUNDLE_INFO_FAILED);
}

/**
 * @tc.name: Bm_Command_QuickFix_Query_1200
 * @tc.desc: Test GetQuickFixInfoString with different type
 * @tc.type: FUNC
 * @tc.require: issueI5OCZV
 */
HWTEST_F(BmCommandQuickFixTest, Bm_Command_QuickFix_Query_1200, TestSize.Level1)
{
    QuickFixCommand command;
    AAFwk::ApplicationQuickFixInfo quickFixInfo;
    std::vector<HqfInfo> hqfInfos;
    HqfInfo hq1;
    hq1.moduleName = "step1";
    hq1.hapSha256 = "step2";
    hq1.hqfFilePath = "step3";
    hqfInfos.emplace_back(hq1);
    quickFixInfo.appqfInfo.hqfInfos = hqfInfos;
    quickFixInfo.appqfInfo.type = AppExecFwk::QuickFixType::PATCH;
    auto ret = command.GetQuickFixInfoString(quickFixInfo);
    EXPECT_EQ(ret, "ApplicationQuickFixInfo:\n  bundle name: \n  bundle version code: 0\n  "
        "bundle version name: \n  patch version code: 0\n  patch version name: \n  "
        "cpu abi: \n  native library path: \n  type: patch\n  ModuelQuickFixInfo:\n    "
        "module name: step1\n    module sha256: step2\n    "
        "file path: step3\n");
    quickFixInfo.appqfInfo.type = AppExecFwk::QuickFixType::HOT_RELOAD;
    ret = command.GetQuickFixInfoString(quickFixInfo);
    EXPECT_EQ(ret, "ApplicationQuickFixInfo:\n  bundle name: \n  bundle version code: 0\n  "
        "bundle version name: \n  patch version code: 0\n  patch version name: \n  "
        "cpu abi: \n  native library path: \n  type: hotreload\n  ModuelQuickFixInfo:\n    "
        "module name: step1\n    module sha256: step2\n    "
        "file path: step3\n");
}
} // namespace AAFwk
} // namespace OHOS
