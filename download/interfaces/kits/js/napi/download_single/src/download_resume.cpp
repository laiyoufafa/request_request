/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "download_resume.h"

#include "download_manager.h"
#include "log.h"
#include "napi_utils.h"

namespace OHOS::Request::Download {
napi_value DownloadResume::Resume(napi_env env, napi_callback_info info)
{
    DOWNLOAD_HILOGD("Resume Enter ---->");
    if (!DownloadManager::GetInstance()->CheckPermission()) {
        DOWNLOAD_HILOGD("no permission to access download service");
        return nullptr;
    }
    return Exec(env, info);
}

napi_value DownloadResume::Restore(napi_env env, napi_callback_info info)
{
    DOWNLOAD_HILOGD("Restore Enter ---->");
    ExceptionError err;
    if (!NapiUtils::CheckParameterCorrect(env, info, FUNCTION_RESTORE, err)) {
        DOWNLOAD_HILOGE("%{public}s", err.errInfo.c_str());
        NapiUtils::ThrowError(env, err.code, err.errInfo);
        return nullptr;
    }
    return Exec(env, info);
}

napi_value DownloadResume::Exec(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<ResumeContext>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0, " should 0 parameter!", napi_invalid_arg);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->result, result);
        DOWNLOAD_HILOGD("output ---- [%{public}s], status[%{public}d]", context->result ? "true" : "false", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->result = DownloadManager::GetInstance()->Resume(context->task_->GetId());
        if (context->result == true) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), "", 0);
    return asyncCall.Call(env, exec);
}
} // namespace OHOS::Request::Download