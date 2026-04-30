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

#include <new>
#include <unistd.h>

#define private public
#include "bundle_test_tool.h"
#undef private

#include "iremote_broker.h"
#include "iremote_object.h"
#include "mock_bundle_installer_host.h"
#include "mock_bundle_mgr_host.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class BundleTestToolCacheStatTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects();
    void SetMockObjects(BundleTestTool &cmd) const;

    sptr<IBundleMgr> mgrProxyPtr_;
    sptr<IBundleInstaller> installerProxyPtr_;
};

void BundleTestToolCacheStatTest::SetUp()
{
    optind = 0;
    MakeMockObjects();
    MockBundleMgrHost::SetGetBundleInfosReturn(true);
    MockBundleMgrHost::SetGetBundleStatsFailSecondBundle(false);
}

void BundleTestToolCacheStatTest::TearDown()
{}

void BundleTestToolCacheStatTest::MakeMockObjects()
{
    auto mgrHostPtr = sptr<IRemoteObject>(new (std::nothrow) MockBundleMgrHost());
    mgrProxyPtr_ = iface_cast<IBundleMgr>(mgrHostPtr);

    auto installerHostPtr = sptr<IRemoteObject>(new (std::nothrow) MockBundleInstallerHost());
    installerProxyPtr_ = iface_cast<IBundleInstaller>(installerHostPtr);
}

void BundleTestToolCacheStatTest::SetMockObjects(BundleTestTool &cmd) const
{
    cmd.bundleMgrProxy_ = mgrProxyPtr_;
    cmd.bundleInstallerProxy_ = installerProxyPtr_;
}

/**
 * @tc.number: Bundle_Test_Tool_Get_Each_Bundle_Cache_Stat_0100
 * @tc.name: GetEachBundleCacheStat
 * @tc.desc: Verify GetEachBundleCacheStat returns all bundle cache sizes when all stats succeed.
 */
HWTEST_F(BundleTestToolCacheStatTest, Bundle_Test_Tool_Get_Each_Bundle_Cache_Stat_0100,
    Function | MediumTest | TestSize.Level1)
{
    char *argv[] = {
        const_cast<char*>("bundle_test_tool"),
        const_cast<char*>("getEachBundleCacheStat"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleTestTool cmd(argc, argv);
    SetMockObjects(cmd);

    std::string msg;
    bool ret = cmd.GetEachBundleCacheStat(100, msg);

    EXPECT_TRUE(ret);
    EXPECT_NE(msg.find("bundleName: com.example.bundle.one, appIndex: 0, cache size: 100"), std::string::npos);
    EXPECT_NE(msg.find("bundleName: com.example.bundle.two, appIndex: 1, cache size: 200"), std::string::npos);
    EXPECT_NE(msg.find("success count: 2"), std::string::npos);
    EXPECT_NE(msg.find("fail count: 0"), std::string::npos);
    EXPECT_NE(msg.find("total cache size: 300"), std::string::npos);
}

/**
 * @tc.number: Bundle_Test_Tool_Get_Each_Bundle_Cache_Stat_0200
 * @tc.name: GetEachBundleCacheStat
 * @tc.desc: Verify GetEachBundleCacheStat reports failure when GetBundleInfos fails.
 */
HWTEST_F(BundleTestToolCacheStatTest, Bundle_Test_Tool_Get_Each_Bundle_Cache_Stat_0200,
    Function | MediumTest | TestSize.Level1)
{
    MockBundleMgrHost::SetGetBundleInfosReturn(false);

    char *argv[] = {
        const_cast<char*>("bundle_test_tool"),
        const_cast<char*>("getEachBundleCacheStat"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleTestTool cmd(argc, argv);
    SetMockObjects(cmd);

    std::string msg;
    bool ret = cmd.GetEachBundleCacheStat(100, msg);

    EXPECT_FALSE(ret);
    EXPECT_NE(msg.find("error: get bundle infos failed"), std::string::npos);
}

/**
 * @tc.number: Bundle_Test_Tool_Get_Each_Bundle_Cache_Stat_0300
 * @tc.name: GetEachBundleCacheStat
 * @tc.desc: Verify GetEachBundleCacheStat keeps partial result when one bundle stats query fails.
 */
HWTEST_F(BundleTestToolCacheStatTest, Bundle_Test_Tool_Get_Each_Bundle_Cache_Stat_0300,
    Function | MediumTest | TestSize.Level1)
{
    MockBundleMgrHost::SetGetBundleStatsFailSecondBundle(true);

    char *argv[] = {
        const_cast<char*>("bundle_test_tool"),
        const_cast<char*>("getEachBundleCacheStat"),
        const_cast<char*>(""),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    BundleTestTool cmd(argc, argv);
    SetMockObjects(cmd);

    std::string msg;
    bool ret = cmd.GetEachBundleCacheStat(100, msg);

    EXPECT_FALSE(ret);
    EXPECT_NE(msg.find("bundleName: com.example.bundle.one, appIndex: 0, cache size: 100"), std::string::npos);
    EXPECT_NE(msg.find("bundleName: com.example.bundle.two, appIndex: 1, error: get bundle stats failed"),
        std::string::npos);
    EXPECT_NE(msg.find("success count: 1"), std::string::npos);
    EXPECT_NE(msg.find("fail count: 1"), std::string::npos);
    EXPECT_NE(msg.find("total cache size: 100"), std::string::npos);
}
}  // namespace OHOS
