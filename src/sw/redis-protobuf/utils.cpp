/**************************************************************************
   Copyright (c) 2019 sewenew

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 *************************************************************************/

#include "utils.h"
#include <sys/stat.h>
#include <cstdio>
#include <dirent.h>
#include <cassert>
#include <cctype>
#include <google/protobuf/util/json_util.h>
#include "errors.h"

namespace {

mode_t file_type(const std::string &file);

}

namespace sw {

namespace redis {

namespace pb {

StringView::StringView(RedisModuleString *str) {
    if (str == nullptr) {
        throw Error("null string");
    }

    _data = RedisModule_StringPtrLen(str, &_size);
}

namespace util {

std::string msg_to_json(const gp::Message &msg) {
    std::string json;
    auto status = gp::util::MessageToJsonString(msg, &json);
    if (!status.ok()) {
        throw Error("failed to parse message to json");
    }

    return json;
}

int32_t sv_to_int32(const StringView &sv) {
    try {
        return std::stoi(std::string(sv.data(), sv.size()));
    } catch (const std::exception &e) {
        throw Error("not int32");
    }
}

int64_t sv_to_int64(const StringView &sv) {
    try {
        return std::stoll(std::string(sv.data(), sv.size()));
    } catch (const std::exception &e) {
        throw Error("not int64");
    }
}

uint32_t sv_to_uint32(const StringView &sv) {
    try {
        // TODO: check if it's overflow
        return std::stoul(std::string(sv.data(), sv.size()));
    } catch (const std::exception &e) {
        throw Error("not uint32");
    }
}

uint64_t sv_to_uint64(const StringView &sv) {
    try {
        return std::stoull(std::string(sv.data(), sv.size()));
    } catch (const std::exception &e) {
        throw Error("not uint64");
    }
}

double sv_to_double(const StringView &sv) {
    try {
        return std::stod(std::string(sv.data(), sv.size()));
    } catch (const std::exception &e) {
        throw Error("not double");
    }
}

float sv_to_float(const StringView &sv) {
    try {
        return std::stof(std::string(sv.data(), sv.size()));
    } catch (const std::exception &e) {
        throw Error("not float");
    }
}

bool sv_to_bool(const StringView &sv) {
    bool b = false;
    auto s = std::string(sv.data(), sv.size());
    // TODO: make it case insensitive
    if (s == "true") {
        b = true;
    } else if (s == "false") {
        b = false;
    } else {
        try {
            auto val = std::stoi(s);
            if (val == 0) {
                b = false;
            } else {
                b = true;
            }
        } catch (const std::exception &e) {
            throw Error("not bool");
        }
    }

    return b;
}

std::string sv_to_string(const StringView &sv) {
    return std::string(sv.data(), sv.size());
}

bool str_case_equal(const StringView &s1, const StringView &s2) {
    if (s1.size() != s2.size()) {
        return false;
    }

    const auto *p1 = s1.data();
    const auto *p2 = s2.data();
    for (std::size_t idx = 0; idx != s1.size(); ++idx) {
        if (static_cast<char>(std::toupper(static_cast<unsigned char>(p1[idx])))
                != static_cast<char>(std::toupper(static_cast<unsigned char>(p2[idx])))) {
            return false;
        }
    }

    return true;
}

}

namespace io {

bool is_regular(const std::string &file) {
    return S_ISREG(file_type(file));
}

bool is_directory(const std::string &file) {
    return S_ISDIR(file_type(file));
}

std::vector<std::string> list_dir(const std::string &path) {
    if (!is_directory(path)) {
        throw Error(path + " is not a directory");
    }

    auto *dir = opendir(path.c_str());
    if (dir == nullptr) {
        throw Error("failed to open directory: " + path);
    }

    std::vector<std::string> files;
    dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;

        // Skip "." and ".."
        if (name == "." || name == "..") {
            continue;
        }

        auto file_path = path + "/" + name;
        if (is_directory(file_path)) {
            auto sub_files = list_dir(file_path);
            files.insert(files.end(), sub_files.begin(), sub_files.end());
        } else {
            files.push_back(file_path);
        }
    }

    closedir(dir);

    return files;
}

std::string extension(const std::string &file) {
    auto pos = file.rfind(".");
    if (pos == std::string::npos) {
        return {};
    }

    return file.substr(pos + 1);
}

void remove_file(const std::string &path) {
    std::remove(path.data());
}

}

}

}

}

namespace {

mode_t file_type(const std::string &file) {
    struct stat buf;
    if (stat(file.c_str(), &buf) < 0) {
        throw sw::redis::pb::Error("failed to get file status: " + file);
    }

    return buf.st_mode;
}

}
