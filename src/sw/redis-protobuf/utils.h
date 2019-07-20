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

#ifndef SEWENEW_REDISPROTOBUF_UTILS_H
#define SEWENEW_REDISPROTOBUF_UTILS_H

#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <google/protobuf/message.h>
#include "module_api.h"

namespace sw {

namespace redis {

namespace pb {

namespace gp = google::protobuf;

using MsgUPtr = std::unique_ptr<gp::Message>;

// By now, not all compilers support std::string_view,
// so we make our own implementation.
class StringView {
public:
    constexpr StringView() noexcept = default;

    constexpr StringView(const char *data, std::size_t size) : _data(data), _size(size) {}

    StringView(const char *data) : _data(data), _size(std::strlen(data)) {}

    StringView(const std::string &str) : _data(str.data()), _size(str.size()) {}

    StringView(RedisModuleString *str);

    constexpr StringView(const StringView &) noexcept = default;

    StringView& operator=(const StringView &) noexcept = default;

    constexpr const char* data() const noexcept {
        return _data;
    }

    constexpr std::size_t size() const noexcept {
        return _size;
    }

private:
    const char *_data = nullptr;
    std::size_t _size = 0;
};

template <typename T>
class Optional {
public:
    Optional() = default;

    Optional(const Optional &) = default;
    Optional& operator=(const Optional &) = default;

    Optional(Optional &&) = default;
    Optional& operator=(Optional &&) = default;

    ~Optional() = default;

    template <typename ...Args>
    explicit Optional(Args &&...args) : _value(true, T(std::forward<Args>(args)...)) {}

    explicit operator bool() const {
        return _value.first;
    }

    T& value() {
        return _value.second;
    }

    const T& value() const {
        return _value.second;
    }

    T* operator->() {
        return &(_value.second);
    }

    const T* operator->() const {
        return &(_value.second);
    }

    T& operator*() {
        return _value.second;
    }

    const T& operator*() const {
        return _value.second;
    }

    void reset() noexcept {
        _value.first = false;
    }

private:
    std::pair<bool, T> _value;
};

namespace util {

std::string msg_to_json(const gp::Message &msg);

int32_t sv_to_int32(const StringView &sv);

int64_t sv_to_int64(const StringView &sv);

uint32_t sv_to_uint32(const StringView &sv);

uint64_t sv_to_uint64(const StringView &sv);

double sv_to_double(const StringView &sv);

float sv_to_float(const StringView &sv);

bool sv_to_bool(const StringView &sv);

std::string sv_to_string(const StringView &sv);

bool str_case_equal(const StringView &s1, const StringView &s2);

}

namespace io {

bool is_regular(const std::string &file);

bool is_directory(const std::string &file);

std::vector<std::string> list_dir(const std::string &path);

std::string extension(const std::string &file);

}

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_UTILS_H
