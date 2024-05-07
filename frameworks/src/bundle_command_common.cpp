/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "bundle_command_common.h"

#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "bundle_mgr_proxy.h"
#ifdef DISTRIBUTED_BUNDLE_FRAMEWORK
#include "distributed_bms_proxy.h"
#endif
#ifdef ACCOUNT_ENABLE
#include "os_account_info.h"
#include "os_account_manager.h"
#endif
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "status_receiver_interface.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
sptr<IBundleMgr> BundleCommandCommon::GetBundleMgrProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        APP_LOGE("failed to get system ability mgr.");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        APP_LOGE("failed to get bundle manager proxy.");
        return nullptr;
    }

    APP_LOGD("get bundle manager proxy success.");
    return iface_cast<IBundleMgr>(remoteObject);
}

#ifdef DISTRIBUTED_BUNDLE_FRAMEWORK
sptr<IDistributedBms> BundleCommandCommon::GetDistributedBundleMgrService()
{
    auto saMgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        APP_LOGE("saMgr is nullptr");
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> remoteObject =
        saMgr->CheckSystemAbility(OHOS::DISTRIBUTED_BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        APP_LOGE("failed to get distributed bms proxy.");
        return nullptr;
    }
    return OHOS::iface_cast<IDistributedBms>(remoteObject);
}
#endif

int32_t BundleCommandCommon::GetCurrentUserId(int32_t userId)
{
    if (userId == Constants::UNSPECIFIED_USERID) {
        std::vector<int> activeIds;
#ifdef ACCOUNT_ENABLE
        int32_t ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
        if (ret != 0) {
            APP_LOGW("QueryActiveOsAccountIds failed! ret = %{public}d.", ret);
            return userId;
        }
#endif
        if (activeIds.empty()) {
            APP_LOGW("QueryActiveOsAccountIds activeIds empty");
            return userId;
        }
        return activeIds[0];
    }
    return userId;
}

std::map<int32_t, std::string> BundleCommandCommon::bundleMessageMap_ = {
    //  error + message
    {
        IStatusReceiver::ERR_INSTALL_INTERNAL_ERROR,
        "error: install internal error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_HOST_INSTALLER_FAILED,
        "error: install host installer failed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_FAILED,
        "error: install parse failed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_VERSION_DOWNGRADE,
        "error: install version downgrade.",
    },
    {
        IStatusReceiver::ERR_INSTALL_VERIFICATION_FAILED,
        "error: install verification failed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_INVALID_SIGNATURE_FILE_PATH,
        "error: signature file path is invalid.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_BAD_BUNDLE_SIGNATURE_FILE,
        "error: cannot open signature file.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_NO_BUNDLE_SIGNATURE,
        "error: no signature file.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_VERIFY_APP_PKCS7_FAIL,
        "error: fail to verify pkcs7 file.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_PROFILE_PARSE_FAIL,
        "error: fail to parse signature file.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_APP_SOURCE_NOT_TRUESTED,
        "error: signature verification failed due to not trusted app source.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_BAD_DIGEST,
        "error: signature verification failed due to not bad digest.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_BUNDLE_INTEGRITY_VERIFICATION_FAILURE,
        "error: signature verification failed due to out of integrity.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_FILE_SIZE_TOO_LARGE,
        "error: signature verification failed due to oversize file.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_BAD_PUBLICKEY,
        "error: signature verification failed due to bad public key.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_BAD_BUNDLE_SIGNATURE,
        "error: signature verification failed due to bad bundle signature.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_NO_PROFILE_BLOCK_FAIL,
        "error: signature verification failed due to no profile block.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_BUNDLE_SIGNATURE_VERIFICATION_FAILURE,
        "error: verify signature failed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_VERIFY_SOURCE_INIT_FAIL,
        "error: signature verification failed due to init source failed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_INCOMPATIBLE_SIGNATURE,
        "error: install incompatible signature info.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_INCONSISTENT_SIGNATURE,
        "error: install sign info inconsistent.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_MODULE_NAME_EMPTY,
        "error: install failed due to hap moduleName is empty.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_MODULE_NAME_DUPLICATE,
        "error: install failed due to hap moduleName duplicate.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_CHECK_HAP_HASH_PARAM,
        "error: install failed due to check hap hash param failed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARAM_ERROR,
        "error: install param error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PERMISSION_DENIED,
        "error: install permission denied.",
    },
    {
        IStatusReceiver::ERR_INSTALL_ENTRY_ALREADY_EXIST,
        "error: install entry already exist.",
    },
    {
        IStatusReceiver::ERR_INSTALL_STATE_ERROR,
        "error: install state error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FILE_PATH_INVALID,
        "error: install file path invalid.",
    },
    {
        IStatusReceiver::ERR_INSTALL_INVALID_HAP_NAME,
        "error: install invalid hap name.",
    },
    {
        IStatusReceiver::ERR_INSTALL_INVALID_BUNDLE_FILE,
        "error: install invalid bundle file.",
    },
    {
        IStatusReceiver::ERR_INSTALL_INVALID_HAP_SIZE,
        "error: install invalid hap size.",
    },
    {
        IStatusReceiver::ERR_INSTALL_GENERATE_UID_ERROR,
        "error: install generate uid error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_INSTALLD_SERVICE_ERROR,
        "error: install installd service error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_BUNDLE_MGR_SERVICE_ERROR,
        "error: install bundle mgr service error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_ALREADY_EXIST,
        "error: install already exist.",
    },
    {
        IStatusReceiver::ERR_INSTALL_BUNDLENAME_NOT_SAME,
        "error: install bundle name not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_VERSIONCODE_NOT_SAME,
        "error: install version code not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_VERSIONNAME_NOT_SAME,
        "error: install version name not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_MINCOMPATIBLE_VERSIONCODE_NOT_SAME,
        "error: install min compatible version code not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_VENDOR_NOT_SAME,
        "error: install vendor not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_RELEASETYPE_TARGET_NOT_SAME,
        "error: install releaseType target not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_RELEASETYPE_NOT_SAME,
        "error: install releaseType not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_RELEASETYPE_COMPATIBLE_NOT_SAME,
        "error: install releaseType compatible not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_VERSION_NOT_COMPATIBLE,
        "error: install version not compatible.",
    },
    {
        IStatusReceiver::ERR_INSTALL_APP_DISTRIBUTION_TYPE_NOT_SAME,
        "error: install distribution type not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_APP_PROVISION_TYPE_NOT_SAME,
        "error: install provision type not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_INVALID_NUMBER_OF_ENTRY_HAP,
        "error: install invalid number of entry hap.",
    },
    {
        IStatusReceiver::ERR_INSTALL_DISK_MEM_INSUFFICIENT,
        "error: install failed due to insufficient disk memory.",
    },
    {
        IStatusReceiver::ERR_INSTALL_GRANT_REQUEST_PERMISSIONS_FAILED,
        "error: install failed due to grant request permissions failed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_UPDATE_HAP_TOKEN_FAILED,
        "error: install failed due to update hap token failed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_SINGLETON_NOT_SAME,
        "error: install failed due to singleton not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_ZERO_USER_WITH_NO_SINGLETON,
        "error: install failed due to zero user can only install singleton app.",
    },
    {
        IStatusReceiver::ERR_INSTALL_CHECK_SYSCAP_FAILED,
        "error: install failed due to check syscap filed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_APPTYPE_NOT_SAME,
        "error: install failed due to apptype not same",
    },
    {
        IStatusReceiver::ERR_INSTALL_TYPE_ERROR,
        "error: install failed due to error bundle type"
    },
    {
        IStatusReceiver::ERR_INSTALL_SDK_INCOMPATIBLE,
        "error: install failed due to older sdk version in the device"
    },
    {
        IStatusReceiver::ERR_INSTALL_SO_INCOMPATIBLE,
        "error: install failed due to native so is incompatible"
    },
    {
        IStatusReceiver::ERR_INSTALL_AN_INCOMPATIBLE,
        "error: install failed due to ark native file is incompatible"
    },
    {
        IStatusReceiver::ERR_INSTALL_URI_DUPLICATE,
        "error: install failed due to uri prefix duplicate",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_UNEXPECTED,
        "error: install parse unexpected.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_MISSING_BUNDLE,
        "error: install parse missing bundle.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_MISSING_ABILITY,
        "error: install parse missing ability.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_NO_PROFILE,
        "error: install parse no profile.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_BAD_PROFILE,
        "error: install parse bad profile.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_PROFILE_PROP_TYPE_ERROR,
        "error: install parse profile prop type error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_PROFILE_MISSING_PROP,
        "error: install parse profile missing prop.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_PROFILE_PROP_CHECK_ERROR,
        "error: install parse profile prop check error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_PERMISSION_ERROR,
        "error: install parse permission error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_RPCID_FAILED,
        "error: install parse syscap error.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_NATIVE_SO_FAILED,
        "error: install parse native so failed.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_AN_FAILED,
        "error: install parse ark native file failed.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_PARAM_ERROR,
        "error: installd param error.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_GET_PROXY_ERROR,
        "error: installd get proxy error.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_CREATE_DIR_FAILED,
        "error: installd create dir failed.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_CREATE_DIR_EXIST,
        "error: installd create dir exist.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_CHOWN_FAILED,
        "error: installd chown failed.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_REMOVE_DIR_FAILED,
        "error: installd remove dir failed.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_EXTRACT_FILES_FAILED,
        "error: installd extract files failed.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_RNAME_DIR_FAILED,
        "error: installd rename dir failed.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_CLEAN_DIR_FAILED,
        "error: installd clean dir failed.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_SET_SELINUX_LABEL_FAILED,
        "error: installd set selinux label failed."
    },
    {
        IStatusReceiver::ERR_UNINSTALL_SYSTEM_APP_ERROR,
        "error: uninstall system app error.",
    },
    {
        IStatusReceiver::ERR_UNINSTALL_KILLING_APP_ERROR,
        "error: uninstall killing app error.",
    },
    {
        IStatusReceiver::ERR_UNINSTALL_INVALID_NAME,
        "error: uninstall invalid name.",
    },
    {
        IStatusReceiver::ERR_UNINSTALL_PARAM_ERROR,
        "error: uninstall param error.",
    },
    {
        IStatusReceiver::ERR_UNINSTALL_PERMISSION_DENIED,
        "error: uninstall permission denied.",
    },
    {
        IStatusReceiver::ERR_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR,
        "error: uninstall bundle mgr service error.",
    },
    {
        IStatusReceiver::ERR_UNINSTALL_MISSING_INSTALLED_BUNDLE,
        "error: uninstall missing installed bundle.",
    },
    {
        IStatusReceiver::ERR_UNINSTALL_MISSING_INSTALLED_MODULE,
        "error: uninstall missing installed module.",
    },
    {
        IStatusReceiver::ERR_FAILED_SERVICE_DIED,
        "error: bundle manager service is died.",
    },
    {
        IStatusReceiver::ERR_FAILED_GET_INSTALLER_PROXY,
        "error: failed to get installer proxy.",
    },
    {
        IStatusReceiver::ERR_USER_NOT_EXIST,
        "error: user not exist.",
    },
    {
        IStatusReceiver::ERR_USER_NOT_INSTALL_HAP,
        "error: user does not install the hap.",
    },
    {
        IStatusReceiver::ERR_OPERATION_TIME_OUT,
        "error: operation time out.",
    },
    {
        IStatusReceiver::ERR_INSTALL_NOT_UNIQUE_DISTRO_MODULE_NAME,
        "error: moduleName is not unique.",
    },
    {
        IStatusReceiver::ERR_INSTALL_INCONSISTENT_MODULE_NAME,
        "error: moduleName is inconsistent.",
    },
    {
        IStatusReceiver::ERR_INSTALL_SINGLETON_INCOMPATIBLE,
        "error: singleton is incompatible with installed app.",
    },
    {
        IStatusReceiver::ERR_INSTALL_DISALLOWED,
        "error: Failed to install the HAP because enterprise device management disallow install.",
    },
    {
        IStatusReceiver::ERR_UNINSTALL_DISALLOWED,
        "error: Failed to uninstall the HAP because enterprise device management disallow uninstall.",
    },
    {
        IStatusReceiver::ERR_INSTALL_DEVICE_TYPE_NOT_SUPPORTED,
        "error: device type is not supported.",
    },
    {
        IStatusReceiver::ERR_INSTALL_PARSE_PROFILE_PROP_SIZE_CHECK_ERROR,
        "error: too large size of string or array type element in the profile.",
    },
    {
        IStatusReceiver::ERR_INSTALL_DEPENDENT_MODULE_NOT_EXIST,
        "error: Failed to install the HAP or HSP because the dependent module does not exist.",
    },
    {
        IStatusReceiver::ERR_INSTALL_SHARE_APP_LIBRARY_NOT_ALLOWED,
        "error: Failed to install the HSP due to the lack of required permission.",
    },
    {
        IStatusReceiver::ERR_INSTALL_COMPATIBLE_POLICY_NOT_SAME,
        "error: compatible policy not same.",
    },
    {
        IStatusReceiver::ERR_INSTALL_FILE_IS_SHARED_LIBRARY,
        "error: Failed to install the HSP because disallow install a shared bundle by hapFilePaths.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_INTERNAL_ERROR,
        "error: internal error of overlay installation.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_INVALID_BUNDLE_NAME,
        "error: invalid bundle name of overlay installation.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_INVALID_MODULE_NAME,
        "error: invalid module name of overlay installation.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_ERROR_HAP_TYPE,
        "error: invalid hap type of overlay installation.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_ERROR_BUNDLE_TYPE,
        "error: service bundle is not supported of overlay installation.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_TARGET_BUNDLE_NAME_MISSED,
        "error: target bundleName is missed of overlay installation.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_TARGET_MODULE_NAME_MISSED,
        "error: target module name is missed of overlay installation.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_TARGET_BUNDLE_NAME_NOT_SAME,
        "error: target bundle name is not same when replace external overlay.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_INTERNAL_EXTERNAL_OVERLAY_EXISTED_SIMULTANEOUSLY,
        "error: internal and external overlay installation cannot be supported.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_TARGET_PRIORITY_NOT_SAME,
        "error: target priority is not same when replace external overlay.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_INVALID_PRIORITY,
        "error: invalid priority of overlay hap.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_INCONSISTENT_VERSION_CODE,
        "error: inconsistent version code of internal overlay installation.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_SERVICE_EXCEPTION,
        "error: service is exception.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_BUNDLE_NAME_SAME_WITH_TARGET_BUNDLE_NAME,
        "error: target bundle name cannot be same with bundle name.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_NO_SYSTEM_APPLICATION_FOR_EXTERNAL_OVERLAY,
        "error: external overlay installation only support preInstall bundle.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_DIFFERENT_SIGNATURE_CERTIFICATE,
        "error:target bundle has different signature certificate with current bundle.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_TARGET_BUNDLE_IS_OVERLAY_BUNDLE,
        "error: target bundle cannot be overlay bundle of external overlay installation.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_TARGET_MODULE_IS_OVERLAY_MODULE,
        "error: target module cannot be overlay module of overlay installation",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_OVERLAY_TYPE_NOT_SAME,
        "error: overlay type is not same.",
    },
    {
        IStatusReceiver::ERR_OVERLAY_INSTALLATION_FAILED_INVALID_BUNDLE_DIR,
        "error: bundle dir is invalid.",
    },
    {
        IStatusReceiver::ERR_INSTALL_ASAN_ENABLED_NOT_SAME,
        "error: install asanEnabled not same",
    },
    {
        IStatusReceiver::ERR_INSTALL_ASAN_ENABLED_NOT_SUPPORT,
        "error: install asan enabled is not support",
    },
    {
        IStatusReceiver::ERR_INSTALL_BUNDLE_TYPE_NOT_SAME,
        "error: install bundleType not same",
    },
    {
        IStatusReceiver::ERR_APPEXECFWK_UNINSTALL_SHARE_APP_LIBRARY_IS_NOT_EXIST,
        "error: shared bundle is not exist",
    },
    {
        IStatusReceiver::ERR_APPEXECFWK_UNINSTALL_SHARE_APP_LIBRARY_IS_RELIED,
        "error: The version of the shared bundle is dependent on other applications.",
    },
    {
        IStatusReceiver::ERR_INSATLL_CHECK_PROXY_DATA_URI_FAILED,
        "error: uri in proxy data is wrong",
    },
    {
        IStatusReceiver::ERR_INSATLL_CHECK_PROXY_DATA_PERMISSION_FAILED,
        "error: apl of required permission in proxy data is too low",
    },
    {
        IStatusReceiver::ERR_INSTALL_FAILED_DEBUG_NOT_SAME,
        "error: install debug type not same",
    },
    {
        IStatusReceiver::ERR_INSTALL_ISOLATION_MODE_FAILED,
        "error: isolationMode does not match the system.",
    },
    {
        IStatusReceiver::ERR_INSTALL_CODE_SIGNATURE_FAILED,
        "error: verify code signature failed.",
    },
    {
        IStatusReceiver::ERR_UNINSTALL_FROM_BMS_EXTENSION_FAILED,
        "error: uninstall bundle from extension failed",
    },
    {
        IStatusReceiver::ERR_INSTALL_ENTERPRISE_BUNDLE_NOT_ALLOWED,
        "error: Failed to install the HAP because an enterprise normal/MDM bundle cannot be installed on \
        non-enterprise device.",
    },
    {
        IStatusReceiver::ERR_INSTALLD_COPY_FILE_FAILED,
        "error: copy file failed",
    },
    {
        IStatusReceiver::ERR_INSTALL_GWP_ASAN_ENABLED_NOT_SAME,
        "error: install GWPAsanEnabled not same",
    },
    {
        IStatusReceiver::ERR_INSTALL_DEBUG_BUNDLE_NOT_ALLOWED,
        "error: debug bundle can only be installed in developer mode",
    },
    {
        IStatusReceiver::ERR_INSTALL_CHECK_ENCRYPTION_FAILED,
        "error: check encryption failed",
    },
    {
        IStatusReceiver::ERR_INSTALL_CODE_SIGNATURE_DELIVERY_FILE_FAILED,
        "error: delivery sign profile failed",
    },
    {
        IStatusReceiver::ERR_INSTALL_CODE_SIGNATURE_REMOVE_FILE_FAILED,
        "error: remove sign profile failed",
    },
    {
        IStatusReceiver::ERR_UNKNOWN,
        "error: unknown.",
    }
};
} // AppExecFwk
} // OHOS