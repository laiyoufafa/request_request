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
#ifndef BACKGROUND_NOTIFICATION_H
#define BACKGROUND_NOTIFICATION_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void BackgroundNotify(uint32_t taskId, pid_t uid, uint8_t action, const char *path, int32_t pathLen, uint32_t percent);

#ifdef __cplusplus
}
#endif

#endif // BACKGROUND_NOTIFICATION_H
