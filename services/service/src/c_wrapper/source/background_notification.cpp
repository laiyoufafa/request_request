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

#include "background_notification.h"

#include "int_wrapper.h"
#include "log.h"
#include "notification.h"
#include "notification_constant.h"
#include "notification_content.h"
#include "notification_helper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"

using namespace OHOS::Notification;
static constexpr uint8_t DOWNLOAD = 0;
void BackgroundNotify(uint32_t taskId, pid_t uid, uint8_t action, const char *path, int32_t pathLen, uint32_t percent)
{
    REQUEST_HILOGE("BackgroundNotify");
    auto requestTemplate = std::make_shared<NotificationTemplate>();
    if (requestTemplate == nullptr) {
        REQUEST_HILOGE("taskId: %{public}d, downloadTemplate is null", taskId);
        return;
    }
    std::string filepath(path, pathLen);
    requestTemplate->SetTemplateName("requestTemplate");
    OHOS::AAFwk::WantParams wantParams;
    wantParams.SetParam("progressValue", OHOS::AAFwk::Integer::Box(percent));
    wantParams.SetParam("fileName", OHOS::AAFwk::String::Box(filepath));
    if (action == DOWNLOAD) {
        wantParams.SetParam("title", OHOS::AAFwk::String::Box("Download"));
    } else {
        wantParams.SetParam("title", OHOS::AAFwk::String::Box("Upload"));
    }
    requestTemplate->SetTemplateData(std::make_shared<OHOS::AAFwk::WantParams>(wantParams));
    auto normalContent = std::make_shared<NotificationNormalContent>();
    if (normalContent == nullptr) {
        REQUEST_HILOGE("taskId: %{public}d, normalContent is null", taskId);
        return;
    }
    auto content = std::make_shared<NotificationContent>(normalContent);
    if (content == nullptr) {
        REQUEST_HILOGE("taskId: %{public}d, content is null", taskId);
        return;
    }
    NotificationRequest req(taskId);
    req.SetCreatorUid(uid);
    req.SetContent(content);
    req.SetTemplate(requestTemplate);
    req.SetSlotType(NotificationConstant::OTHER);
    OHOS::ErrCode errCode = NotificationHelper::PublishNotification(req);
    if (errCode != OHOS::ERR_OK) {
        REQUEST_HILOGE("notification errCode: %{public}d", errCode);
    }
}
