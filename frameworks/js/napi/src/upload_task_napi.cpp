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

#include "upload_task_napi.h"
#include <uv.h>
#include "upload/async_call.h"
#include "js_util.h"
#include "upload_task.h"
#include "ability.h"
#include "napi_base_context.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::Request::Upload;
namespace OHOS::Request::UploadNapi {
std::map<std::string, UploadTaskNapi::Exec> UploadTaskNapi::onTypeHandlers_ = {
    {"progress", UploadTaskNapi::OnProgress},
    {"headerReceive", UploadTaskNapi::OnHeaderReceive},
    {"fail", UploadTaskNapi::OnFail},
    {"complete", UploadTaskNapi::OnComplete},
};
std::map<std::string, UploadTaskNapi::Exec> UploadTaskNapi::offTypeHandlers_ = {
    {"progress", UploadTaskNapi::OffProgress},
    {"headerReceive", UploadTaskNapi::OffHeaderReceive},
    {"fail", UploadTaskNapi::OffFail},
    {"complete", UploadTaskNapi::OffComplete},
};

napi_value UploadTaskNapi::JsUpload(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter JsUpload.");
    struct ContextInfo {
        napi_ref ref = nullptr;
    };
    auto ctxInfo = std::make_shared<ContextInfo>();
    auto input = [ctxInfo](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Upload parser to native params %{public}d!", static_cast<int>(argc));
        NAPI_ASSERT_BASE(env, (argc > 0) && (argc <= 2), " need 1 or 2 parameters!", napi_invalid_arg);
        napi_value uploadProxy = nullptr;
        napi_status status = napi_new_instance(env, GetCtor(env), argc, argv, &uploadProxy);
        if ((uploadProxy == nullptr) || (status != napi_ok)) {
            return napi_generic_failure;
        }
        napi_create_reference(env, uploadProxy, 1, &(ctxInfo->ref));
        return napi_ok;
    };
    auto output = [ctxInfo](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_reference_value(env, ctxInfo->ref, result);
        napi_delete_reference(env, ctxInfo->ref);
        return status;
    };
    auto context = std::make_shared<AsyncCall::Context>(input, output);
    AsyncCall asyncCall(env, info, context);
    return asyncCall.Call(env);
}

napi_status UploadTaskNapi::ParseParam(napi_env env, napi_callback_info info, bool IsRequiredParam,
    JsParam &jsParam)
{
    size_t argc = JSUtil::MAX_ARGC;
    napi_value argv[JSUtil::MAX_ARGC] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsParam.self, nullptr);
    if (status != napi_ok) {
        return napi_invalid_arg;
    }
    if (jsParam.self == nullptr) {
        return napi_invalid_arg;
    }

    if (!JSUtil::CheckParamNumber(argc, IsRequiredParam)) {
        return napi_invalid_arg;
    }
    if (!JSUtil::CheckParamType(env, argv[0], napi_string)) {
        return napi_invalid_arg;
    }
    jsParam.type = JSUtil::Convert2String(env, argv[0]);
    if (onTypeHandlers_.find(jsParam.type) == onTypeHandlers_.end()) {
        return napi_invalid_arg;
    }
    if (argc == TWO_ARG) {
        if (!JSUtil::CheckParamType(env, argv[1], napi_function)) {
            return napi_invalid_arg;
        }
        jsParam.callback = argv[1];
    }
    return napi_ok;
}

napi_value UploadTaskNapi::JsOn(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter JsOn.");
    bool IsRequiredParam = true;
    JsParam jsParam;
    napi_status status = ParseParam(env, info, IsRequiredParam, jsParam);
    NAPI_ASSERT(env, status == napi_ok, "ParseParam fail");
    auto handle = onTypeHandlers_.find(jsParam.type);
    handle->second(env, jsParam.callback, jsParam.self);
    return nullptr;
}

napi_value UploadTaskNapi::JsOff(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter JsOff.");
    bool IsRequiredParam = false;
    JsParam jsParam;
    napi_status status = ParseParam(env, info, IsRequiredParam, jsParam);
    NAPI_ASSERT(env, status == napi_ok, "ParseParam fail");
    auto handle = offTypeHandlers_.find(jsParam.type);
    handle->second(env, jsParam.callback, jsParam.self);
    return nullptr;
}

napi_value UploadTaskNapi::JsRemove(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter JsRemove.");
    auto context = std::make_shared<RemoveContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0, " should 0 parameter!", napi_invalid_arg);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->removeStatus, result);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->removeStatus = context->proxy->napiUploadTask_->Remove();
        if (context->removeStatus == true) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context));
    return asyncCall.Call(env, exec);
}

napi_status UploadTaskNapi::OnProgress(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OnProgress.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    std::shared_ptr<IProgressCallback> progressCallback = std::make_shared<ProgressCallback>(env, callback);
    if (JSUtil::Equals(env, callback, progressCallback->GetCallback()) && proxy->onProgress_ != nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnProgress callback already register!");
        return napi_generic_failure;
    }

    proxy->napiUploadTask_->On(TYPE_PROGRESS_CALLBACK, (void *)(progressCallback.get()));
    proxy->onProgress_ = std::move(progressCallback);
    return napi_ok;
}

napi_status UploadTaskNapi::OnHeaderReceive(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OnHeaderReceive.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    std::shared_ptr<IHeaderReceiveCallback> headerReceiveCallback =
                                            std::make_shared<HeaderReceiveCallback>(env, callback);
    if (JSUtil::Equals(env, callback, headerReceiveCallback->GetCallback()) && proxy->onHeaderReceive_ != nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnHeaderReceive callback already register!");
        return napi_generic_failure;
    }

    proxy->napiUploadTask_->On(TYPE_HEADER_RECEIVE_CALLBACK, (void *)(headerReceiveCallback.get()));
    proxy->onHeaderReceive_ = std::move(headerReceiveCallback);
    return napi_ok;
}

napi_status UploadTaskNapi::OnFail(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OnFail.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    std::shared_ptr<INotifyCallback> failCallback = std::make_shared<NotifyCallback>(env, callback);
    if (JSUtil::Equals(env, callback, failCallback->GetCallback()) && proxy->onFail_ != nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnFail callback already register!");
        return napi_generic_failure;
    }

    proxy->napiUploadTask_->On(TYPE_FAIL_CALLBACK, (void *)(failCallback.get()));
    proxy->onFail_ = std::move(failCallback);
    return napi_ok;
}

napi_status UploadTaskNapi::OnComplete(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OnComplete.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    std::shared_ptr<INotifyCallback> completeCallback = std::make_shared<NotifyCallback>(env, callback);
    if (JSUtil::Equals(env, callback, completeCallback->GetCallback()) && proxy->onComplete_ != nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnComplete callback already register!");
        return napi_generic_failure;
    }

    proxy->napiUploadTask_->On(TYPE_COMPLETE_CALLBACK, (void *)(completeCallback.get()));
    proxy->onComplete_ = std::move(completeCallback);
    return napi_ok;
}

napi_status UploadTaskNapi::OffProgress(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OffProgress.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    if (proxy->onProgress_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Progress. proxy->onProgress_ == nullptr.");
        return napi_generic_failure;
    } else {
        std::shared_ptr<IProgressCallback> progressCallback = std::make_shared<ProgressCallback>(env, callback);
        proxy->napiUploadTask_->Off(TYPE_PROGRESS_CALLBACK, (void *)(progressCallback.get()));
        proxy->onProgress_ = nullptr;
    }
    return napi_ok;
}

napi_status UploadTaskNapi::OffHeaderReceive(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OffHeaderReceive.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    if (proxy->onHeaderReceive_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "HeaderReceive. proxy->onHeaderReceive_ == nullptr.");
        return napi_generic_failure;
    } else {
        std::shared_ptr<IHeaderReceiveCallback> headerReceiveCallback =
                                                std::make_shared<HeaderReceiveCallback>(env, callback);
        proxy->napiUploadTask_->Off(TYPE_HEADER_RECEIVE_CALLBACK, (void *)(headerReceiveCallback.get()));
        proxy->onHeaderReceive_ = nullptr;
    }
    return napi_ok;
}


napi_status UploadTaskNapi::OffFail(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OffFail.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);

    if (proxy->onFail_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Fail. proxy->onFail_ == nullptr.");
        return napi_generic_failure;
    } else {
        std::shared_ptr<INotifyCallback> failCallback = std::make_shared<NotifyCallback>(env, callback);
        proxy->napiUploadTask_->Off(TYPE_FAIL_CALLBACK, failCallback.get());
        proxy->onFail_ = nullptr;
    }
    return napi_ok;
}


napi_status UploadTaskNapi::OffComplete(napi_env env, napi_value callback, napi_value self)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Enter OffComplete.");
    UploadTaskNapi *proxy = nullptr;
    NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
    NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);
    if (proxy->onComplete_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "CompleteCallback. proxy->OffComplete_ == nullptr.");
        return napi_generic_failure;
    } else {
        std::shared_ptr<INotifyCallback> completeCallback = std::make_shared<NotifyCallback>(env, callback);
        proxy->napiUploadTask_->Off(TYPE_COMPLETE_CALLBACK, completeCallback.get());
        proxy->onComplete_ = nullptr;
    }
    return napi_ok;
}

UploadTaskNapi &UploadTaskNapi::operator=(std::shared_ptr<Upload::UploadTask> &&uploadTask)
{
    if (napiUploadTask_ == uploadTask) {
        return *this;
    }
    napiUploadTask_ = std::move(uploadTask);
    return *this;
}

bool UploadTaskNapi::operator==(const std::shared_ptr<Upload::UploadTask> &uploadTask)
{
    return napiUploadTask_ == uploadTask;
}

void AddCallbackToConfig(const std::shared_ptr<UploadConfig> &config, napi_env env, napi_value jsConfig,
    UploadTaskNapi *proxy)
{
    config->fsuccess = std::bind(&UploadTaskNapi::OnSystemSuccess, proxy->env_, proxy->success_,
        std::placeholders::_1);
    config->ffail = std::bind(&UploadTaskNapi::OnSystemFail, proxy->env_, proxy->fail_,
        std::placeholders::_1, std::placeholders::_2);
    config->fcomplete = std::bind(&UploadTaskNapi::OnSystemComplete, proxy->env_, proxy->complete_);
}

napi_value UploadTaskNapi::GetCtor(napi_env env)
{
    napi_value cons = nullptr;
    napi_property_descriptor clzDes[] = {
        DECLARE_NAPI_METHOD("on", JsOn),
        DECLARE_NAPI_METHOD("off", JsOff),
        DECLARE_NAPI_METHOD("remove", JsRemove),
    };
    NAPI_CALL(env, napi_define_class(env, "UploadTaskNapi", NAPI_AUTO_LENGTH, Initialize, nullptr,
                                     sizeof(clzDes) / sizeof(napi_property_descriptor), clzDes, &cons));
    return cons;
}

napi_value UploadTaskNapi::Initialize(napi_env env, napi_callback_info info)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "constructor upload task!");
    napi_value self = nullptr;
    size_t argc = JSUtil::MAX_ARGC;
    int parametersPosition = 0;
    napi_value argv[JSUtil::MAX_ARGC] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));
    auto *proxy = new (std::nothrow) UploadTaskNapi();
    if (proxy == nullptr) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Failed to create UploadTaskNapi");
        NAPI_ASSERT(env, false, "Failed to create UploadTaskNapi");
        return nullptr;
    }
    proxy->env_ = env;
    std::shared_ptr<OHOS::AbilityRuntime::Context> context = nullptr;
    napi_status getStatus = GetContext(env, &argv[0], parametersPosition, context);
    if (getStatus != napi_ok) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Initialize. GetContext fail.");
        delete proxy;
        NAPI_ASSERT(env, false, "Initialize. GetContext fail");
        return nullptr;
    }
    std::string version;
    SetVersion(env, argv[parametersPosition], proxy, version);
    proxy->napiUploadConfig_ = JSUtil::ParseUploadConfig(env, argv[parametersPosition], version);
    if (proxy->napiUploadConfig_ == nullptr) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Initialize. ParseUploadConfig fail.");
        delete proxy;
        NAPI_ASSERT(env, false, "Initialize. ParseUploadConfig fail");
        return nullptr;
    }
    if (proxy->napiUploadConfig_->protocolVersion == API5) {
        AddCallbackToConfig(proxy->napiUploadConfig_, env, argv[parametersPosition], proxy);
    }
    proxy->napiUploadTask_ = std::make_shared<Upload::UploadTask>(proxy->napiUploadConfig_);
    proxy->napiUploadTask_->SetContext(context);
    proxy->napiUploadTask_->ExecuteTask();
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Initialize. GetAndSetContext[%{public}d]", getStatus);
    auto finalize = [](napi_env env, void * data, void * hint) {
        UploadTaskNapi *proxy = reinterpret_cast<UploadTaskNapi *>(data);
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "UploadTaskNapi. delete.");
        proxy->napiUploadTask_->Remove();
        delete proxy;
    };
    UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "UploadTaskNapi. napi_wrap OK.");
    if (napi_wrap(env, self, proxy, finalize, nullptr, nullptr) != napi_ok) {
        finalize(env, proxy, nullptr);
        NAPI_ASSERT(env, false, "napi_wrap fail");
        return nullptr;
    }
    return self;
}

void UploadTaskNapi::SetVersion(napi_env env, napi_value jsConfig, UploadTaskNapi *proxy, std::string &version)
{
    if ((JSUtil::ParseFunction(env, jsConfig, "success", proxy->success_)) ||
        (JSUtil::ParseFunction(env, jsConfig, "fail", proxy->fail_)) ||
        (JSUtil::ParseFunction(env, jsConfig, "complete", proxy->complete_))) {
        version = API5;
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "config.protocolVersion = API5");
    }
}

napi_status UploadTaskNapi::GetContext(napi_env env, napi_value *argv, int& parametersPosition,
    std::shared_ptr<OHOS::AbilityRuntime::Context>& context)
{
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, argv[0], stageMode);
    if (status != napi_ok) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "GetAndSetContext. API8");
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "GetAndSetContext. API8. GetCurrentAbility ability == nullptr.");
            return napi_generic_failure;
        }
        context = ability->GetAbilityContext();
    } else {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "GetAndSetContext. API9");
        parametersPosition = 1;
        if (stageMode) {
            context = OHOS::AbilityRuntime::GetStageModeContext(env, argv[0]);
            if (context == nullptr) {
                UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI,
                    "GetAndSetContext. API9. GetStageModeContext contextRtm == nullptr.");
                return napi_generic_failure;
            }
        } else {
            auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
            if (ability == nullptr) {
                UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "GetAndSetContext. API9. GetCurrentAbility ability == nullptr.");
                return napi_generic_failure;
            }
            context = ability->GetAbilityContext();
        }
    }
    if (context == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "GetAndSetContext failed. context is nullptr.");
        return napi_generic_failure;
    }
    return napi_ok;
}

void UploadTaskNapi::OnSystemSuccess(napi_env env, napi_ref ref, Upload::UploadResponse &response)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemSuccess enter");
    uv_loop_s *loop_ = nullptr;
    napi_get_uv_event_loop(env, &loop_);
    if (loop_ == nullptr) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Failed to get uv event loop");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create uv work");
        return;
    }
    SystemSuccessCallback *successCallback = new (std::nothrow)SystemSuccessCallback;
    if (successCallback == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create successCallback");
        delete work;
        return;
    }
    successCallback->env = env;
    successCallback->ref = ref;
    successCallback->response = response;
    work->data = (void *)successCallback;
    int ret = uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            SystemSuccessCallback *successCallback = reinterpret_cast<SystemSuccessCallback *>(work->data);
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(successCallback->env, &scope);
            napi_value callback = nullptr;
            napi_value global = nullptr;
            napi_value result = nullptr;
            napi_value jsResponse = JSUtil::Convert2JSUploadResponse(successCallback->env, successCallback->response);
            napi_value args[1] = { jsResponse };
            napi_get_reference_value(successCallback->env, successCallback->ref, &callback);
            napi_get_global(successCallback->env, &global);
            napi_call_function(successCallback->env, global, callback, 1, args, &result);
            napi_delete_reference(successCallback->env, successCallback->ref);
            napi_close_handle_scope(successCallback->env, scope);
            delete successCallback;
            successCallback = nullptr;
            delete work;
            work = nullptr;
        });
    if (ret != 0) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "OnSystemSuccess. uv_queue_work Failed");
        delete successCallback;
        delete work;
    }
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemSuccess end");
}

void UploadTaskNapi::OnSystemFail(napi_env env, napi_ref ref, std::string &data, int32_t &code)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemFail enter");
    uv_loop_s *loop_ = nullptr;
    napi_get_uv_event_loop(env, &loop_);
    if (loop_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to get uv event loop");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create uv work");
        return;
    }
    SystemFailCallback *failCallback = new (std::nothrow) SystemFailCallback;
    failCallback->data = data;
    failCallback->code = code;
    failCallback->env = env;
    failCallback->ref = ref;
    work->data = (void *)failCallback;
    int ret = uv_queue_work(loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            SystemFailCallback *failCallback = reinterpret_cast<SystemFailCallback *>(work->data);
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(failCallback->env, &scope);
            napi_value callback = nullptr;
            napi_value global = nullptr;
            napi_value result = nullptr;
            napi_value jsData = nullptr;
            napi_create_string_utf8(failCallback->env, failCallback->data.c_str(), failCallback->data.size(), &jsData);
            napi_value jsCode = nullptr;
            napi_create_int32(failCallback->env, failCallback->code, &jsCode);
            napi_value args[2] = { jsData, jsCode };
            napi_get_reference_value(failCallback->env, failCallback->ref, &callback);
            napi_get_global(failCallback->env, &global);
            napi_call_function(failCallback->env, global, callback, sizeof(args) / sizeof(args[0]), args, &result);
            napi_delete_reference(failCallback->env, failCallback->ref);
            napi_close_handle_scope(failCallback->env, scope);
            delete failCallback;
            failCallback = nullptr;
            delete work;
            work = nullptr;
        });
    if (ret != 0) {
        delete failCallback;
        delete work;
    }
}

void UploadTaskNapi::OnSystemComplete(napi_env env, napi_ref ref)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemComplete enter");
    uv_loop_s *loop_ = nullptr;
    napi_get_uv_event_loop(env, &loop_);
    if (loop_ == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to get uv event loop");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create uv work");
        return;
    }
    SystemCompleteCallback *completeCallback = new (std::nothrow)SystemCompleteCallback;
    completeCallback->env = env;
    completeCallback->ref = ref;
    work->data = (void *)completeCallback;
    int ret = uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            SystemCompleteCallback *completeCallback = reinterpret_cast<SystemCompleteCallback *>(work->data);
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(completeCallback->env, &scope);
            napi_value callback = nullptr;
            napi_value global = nullptr;
            napi_value result = nullptr;

            napi_get_reference_value(completeCallback->env, completeCallback->ref, &callback);
            napi_get_global(completeCallback->env, &global);
            napi_call_function(completeCallback->env, global, callback, 0, nullptr, &result);
            napi_delete_reference(completeCallback->env, completeCallback->ref);
            napi_close_handle_scope(completeCallback->env, scope);
            delete completeCallback;
            completeCallback = nullptr;
            delete work;
            work = nullptr;
        });
    if (ret != 0) {
        delete completeCallback;
        delete work;
    }
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "OnSystemComplete end");
}
} // namespace OHOS::Request::UploadNapi