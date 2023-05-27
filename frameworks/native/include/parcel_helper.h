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

#ifndef REQUEST_PARCEL_HELPER_H
#define REQUEST_PARCEL_HELPER_H
#include "js_common.h"
#include "message_parcel.h"
#include "visibility.h"
namespace OHOS {
namespace Request {
class ParcelHelper {
public:
    REQUEST_API static void UnMarshal(MessageParcel &data, TaskInfo &info);
};
} // namespace Request
} // namespace OHOS
#endif //REQUEST_PARCEL_HELPER_H