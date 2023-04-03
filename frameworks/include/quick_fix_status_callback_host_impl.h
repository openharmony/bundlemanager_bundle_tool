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

#ifndef FOUNDATION_APPEXECFWK_STANDARD_TOOLS_BM_INCLUDE_QUICK_FIX_STATUS_CALLBACK_HOST_IMPL_H
#define FOUNDATION_APPEXECFWK_STANDARD_TOOLS_BM_INCLUDE_QUICK_FIX_STATUS_CALLBACK_HOST_IMPL_H

#include <future>

#include "appexecfwk_errors.h"
#include "quick_fix_status_callback_host.h"

namespace OHOS {
namespace AppExecFwk {
class QuickFixStatusCallbackHostlmpl : public QuickFixStatusCallbackHost {
public:
    QuickFixStatusCallbackHostlmpl();
    virtual ~QuickFixStatusCallbackHostlmpl() override;

    virtual void OnPatchDeployed(const std::shared_ptr<QuickFixResult> &result) override;
    virtual void OnPatchSwitched(const std::shared_ptr<QuickFixResult> &result) override;
    virtual void OnPatchDeleted(const std::shared_ptr<QuickFixResult> &result) override;

    ErrCode GetResultCode(std::shared_ptr<QuickFixResult> &quickFixRes) const;

private:
    mutable std::promise<std::shared_ptr<QuickFixResult>> quickFixPromise_;

    DISALLOW_COPY_AND_MOVE(QuickFixStatusCallbackHostlmpl);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // FOUNDATION_APPEXECFWK_STANDARD_TOOLS_BM_INCLUDE_QUICK_FIX_STATUS_CALLBACK_HOST_IMPL_H