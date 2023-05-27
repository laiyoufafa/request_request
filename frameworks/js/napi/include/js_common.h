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

#ifndef JS_COMMON_H
#define JS_COMMON_H

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include "constant.h"

namespace OHOS::Request {

enum class Action : uint32_t {
    DOWNLOAD,
    UPLOAD,
};

enum class Mode : uint32_t {
    BACKGROUND,
    FOREGROUND,
};

enum class Network : uint32_t {
    ANY,
    WIFI,
    CELLULAR,
};

enum class Version : uint32_t {
    API8,
    API9,
    API10,
};

enum Reason : uint32_t {
    REASON_OK,
    TASK_SURVIVAL_ONE_MONTH,
    WAITTING_NETWORK_ONE_DAY,
    STOPPED_NEW_FRONT_TASK,
    RUNNING_TASK_MEET_LIMITS,
    USER_OPERATION,
    APP_BACKGROUND_OR_TERMINATE,
    NETWORK_OFFLINE,
    UNSUPPORTED_NETWORK_TYPE,
    BUILD_CLIENT_FAILED,
    BUILD_REQUEST_FAILED,
    GET_FILESIZE_FAILED,
    CONTINUOUS_TASK_TIMEOUT,
    CONNECT_ERROR,
    REQUEST_ERROR,
    UPLOAD_FILE_ERROR,
    REDIRECT_ERROR,
    PROTOCOL_ERROR,
    IO_ERROR,
    OTHERS_ERROR,
};

struct UploadResponse {
    int32_t code;
    std::string data;
    std::string headers;
};

struct FormItem {
    std::string name;
    std::string value;
};

struct FileSpec {
    std::string name;
    std::string uri;
    std::string filename;
    std::string type;
    int32_t fd = -1;
};

struct Config {
    Action action;
    std::string url;
    Version version;
    std::string bundleName;
    Mode mode = Mode::BACKGROUND;
    Network network = Network::ANY;
    uint32_t index = 0;
    int64_t begins = 0;
    int64_t ends = -1;
    bool overwrite = false;
    bool metered = false;
    bool roaming = false;
    bool retry = true;
    bool redirect = true;
    bool gauge = false;
    bool precise = false;
    bool background = false;
    bool withErrCode = true;
    std::string title;
    std::string saveas;
    std::string method;
    std::string token;
    std::string description;
    std::string data;
    std::map<std::string, std::string> headers;
    std::vector<FormItem> forms;
    std::vector<FileSpec> files;
    std::map<std::string, std::string> extras;
};

enum class State : uint32_t {
    INITIALIZED = 0x00,
    WAITING = 0x10,
    RUNNING = 0x20,
    RETRYING = 0x21,
    PAUSED = 0x30,
    STOPPED = 0x31,
    COMPLETED = 0x40,
    FAILED = 0x41,
    REMOVED = 0x50,
    DEFAULT = 0x60,
};

struct Progress {
    State state;
    uint32_t index;
    uint64_t processed;
    uint64_t totalProcessed;
    std::vector<int64_t> sizes;
    std::map<std::string, std::string> extras;
};

enum class Broken : uint32_t {
    OTHERS = 0xFF,
    DISCONNECTED = 0x00,
    TIMEOUT = 0x10,
    PROTOCOL = 0x20,
    FSIO = 0x40,
};

struct TaskState {
    std::string path;
    uint32_t responseCode {REASON_OK};
    std::string message;
};

struct NotifyData {
    Progress progress;
    Action action;
    Version version;
    Mode mode;
    std::vector<TaskState> taskStates;
};

enum EventType : uint32_t {
    DATA_CALLBACK,
    HEADER_CALLBACK,
    TASK_STATE_CALLBACK,
    PROGRESS_CALLBACK,
};

struct Notify {
    EventType type;
    std::vector<int64_t> data;
    std::map<std::string, std::string> header;
    std::vector<TaskState> taskStates;
    Progress progress;
};

struct TaskInfo {
    Version version;
    std::string uid;
    std::string bundle;
    std::string url;
    std::string data;
    std::vector<FileSpec> files;
    std::vector<FormItem> forms;
    std::string tid;
    std::string title;
    std::string description;
    Action action;
    Mode mode;
    std::string mimeType;
    Progress progress;
    bool gauge;
    std::string ctime;
    std::string mtime;
    bool retry;
    uint32_t tries;
    Broken broken;
    Reason code;
    std::string reason;
    std::map<std::string, std::string> extras;
    std::vector<TaskState> taskStates;
};

struct Filter {
    std::string bundle;
    std::string before;
    std::string after;
    State state;
    Action action;
    Mode mode;
};

enum DownloadErrorCode {
    ERROR_CANNOT_RESUME,
    ERROR_DEVICE_NOT_FOUND,
    ERROR_FILE_ALREADY_EXISTS,
    ERROR_FILE_ERROR,
    ERROR_HTTP_DATA_ERROR,
    ERROR_INSUFFICIENT_SPACE,
    ERROR_TOO_MANY_REDIRECTS,
    ERROR_UNHANDLED_HTTP_CODE,
    ERROR_UNKNOWN,
    ERROR_OFFLINE,
    ERROR_UNSUPPORTED_NETWORK_TYPE,
};

enum DownloadStatus {
    SESSION_SUCCESS,
    SESSION_RUNNING,
    SESSION_PENDING,
    SESSION_PAUSED,
    SESSION_FAILED,
    SESSION_UNKNOWN,
};

struct DownloadInfo {
    uint32_t downloadId;
    DownloadErrorCode failedReason;
    std::string fileName;
    std::string filePath;
    PausedReason pausedReason;
    DownloadStatus status;
    std::string url;
    std::string downloadTitle;
    int64_t downloadTotalBytes;
    std::string description;
    int64_t downloadedBytes;
};
}
#endif //JS_COMMON_H