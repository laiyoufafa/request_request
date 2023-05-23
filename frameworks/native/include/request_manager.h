/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <map>
#include <mutex>
#include <condition_variable>

#include "data_ability_helper.h"
#include "iremote_object.h"
#include "refbase.h"
#include "visibility.h"
#include "constant.h"
#include "notify_stub.h"
#include "request_service_interface.h"
#include "js_common.h"

namespace OHOS::Request {
class RequestSaDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit RequestSaDeathRecipient();
    ~RequestSaDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};

class RequestManager : public RefBase {
public:
    RequestManager();
    ~RequestManager();
    REQUEST_API static sptr<RequestManager> GetInstance();
    REQUEST_API int32_t Create(const Config &config, int32_t &err);
    REQUEST_API int32_t Start(const std::string &tid);
    REQUEST_API int32_t Stop(const std::string &tid);
    REQUEST_API int32_t Show(const std::string &tid, TaskInfo &info);
    REQUEST_API int32_t Touch(const std::string &tid, const std::string &token, TaskInfo &info);
    REQUEST_API int32_t Search(const Filter &filter, std::vector<std::string> &tids);
    REQUEST_API int32_t Query(const std::string &tid, TaskInfo &info);
    REQUEST_API int32_t Clear(const std::vector<std::string> &tids, std::vector<std::string> &res);
    REQUEST_API int32_t Pause(const std::string &tid);
    REQUEST_API int32_t QueryMimeType(const std::string &tid, std::string &mimeType);
    REQUEST_API int32_t Remove(const std::string &tid);
    REQUEST_API int32_t Resume(const std::string &tid);

    REQUEST_API int32_t On(const std::string &type, const std::string &tid,
        const sptr<NotifyInterface> &listener);
    REQUEST_API int32_t Off(const std::string &type, const std::string &tid);
    
    void OnRemoteSaDied(const wptr<IRemoteObject> &object);
    REQUEST_API bool LoadRequestServer();
    void LoadServerSuccess();
    void LoadServerFail();
private:
    sptr<RequestServiceInterface> GetRequestServiceProxy();
    int32_t Retry(int32_t &taskId, const Config &config, int32_t errorCode);
    void SetRequestServiceProxy(sptr<RequestServiceInterface> proxy);

private:
    static std::mutex instanceLock_;
    static sptr<RequestManager> instance_;
    std::mutex downloadMutex_;
    std::mutex conditionMutex_;
    std::mutex serviceProxyMutex_;

    sptr<RequestServiceInterface> requestServiceProxy_;
    sptr<RequestSaDeathRecipient> deathRecipient_;
    std::condition_variable syncCon_;
    bool ready_ = false;
    static constexpr int LOAD_SA_TIMEOUT_MS = 15000;
};
} // namespace OHOS::Request
#endif // DOWNLOAD_MANAGER_H
