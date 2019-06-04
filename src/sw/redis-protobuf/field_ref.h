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

#ifndef SEWENEW_REDISPROTOBUF_FIELD_REF_H
#define SEWENEW_REDISPROTOBUF_FIELD_REF_H

#include <string>
#include <vector>
#include <google/protobuf/message.h>
#include "module_api.h"
#include "utils.h"

namespace sw {

namespace redis {

namespace pb {

namespace gp = google::protobuf;

class Path {
public:
    explicit Path(const StringView &str);

    Path(const Path &) = default;
    Path& operator=(const Path &) = default;

    Path(Path &&) = default;
    Path& operator=(Path &&) = default;

    ~Path() = default;

    const std::string& type() const {
        return _type;
    }

    const std::vector<std::string> fields() const {
        return _fields;
    }

    bool empty() const {
        return _fields.empty();
    }

private:
    std::string _parse_type(const char *ptr, std::size_t len);

    std::vector<std::string> _parse_fields(const char *ptr, std::size_t len);

    std::string _type;

    std::vector<std::string> _fields;
};

class FieldRef {
public:
    FieldRef(gp::Message *root_msg, const Path &path);

    gp::FieldDescriptor::CppType type() const;

    const gp::Message* msg() const {
        return _msg;
    }

    gp::Message* msg() {
        return _msg;
    }

    bool is_array() const {
        return _field_desc != nullptr && _field_desc->is_repeated();
    }

    bool is_array_element() const {
        return _arr_idx >= 0;
    }

    bool is_map() const {
        return _field_desc != nullptr && _field_desc->is_map();
    }

    explicit operator bool() const {
        return _field_desc != nullptr;
    }

    void set_int32(int32_t val);
    void set_int64(int64_t val);
    void set_uint32(uint32_t val);
    void set_uint64(uint64_t val);
    void set_float(float val);
    void set_double(double val);
    void set_bool(bool val);
    void set_string(const std::string &val);
    void set_msg(gp::Message &msg);

    int32_t get_int32() const;
    int64_t get_int64() const;
    uint32_t get_uint32() const;
    uint64_t get_uint64() const;
    float get_float() const;
    double get_double() const;
    bool get_bool() const;
    std::string get_string() const;
    const gp::Message& get_msg() const;

    void set_repeated_int32(int32_t val);
    void set_repeated_int64(int64_t val);
    void set_repeated_uint32(uint32_t val);
    void set_repeated_uint64(uint64_t val);
    void set_repeated_float(float val);
    void set_repeated_double(double val);
    void set_repeated_bool(bool val);
    void set_repeated_string(const std::string &val);
    void set_repeated_msg(gp::Message &msg);

    int32_t get_repeated_int32() const;
    int64_t get_repeated_int64() const;
    uint32_t get_repeated_uint32() const;
    uint64_t get_repeated_uint64() const;
    float get_repeated_float() const;
    double get_repeated_double() const;
    bool get_repeated_bool() const;
    std::string get_repeated_string() const;
    const gp::Message& get_repeated_msg() const;

    void clear();

private:
    gp::Message *_msg = nullptr;

    const gp::FieldDescriptor *_field_desc = nullptr;

    int _arr_idx = -1;

    enum class ParentType {
        MSG,
        ARR,
        MAP,
        SCALAR,
        INVALID
    };

    void _validate_parameters(gp::Message *root_msg, const Path &path) const;

    void _parse_aggregate_field(const std::string &field);
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_FIELD_REF_H
