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

struct FieldRef {
public:
    FieldRef(gp::Message *parent_msg, const Path &path);

    gp::FieldDescriptor::CppType type() const;

    explicit operator bool() const {
        return field_desc != nullptr;
    }

    gp::Message *msg = nullptr;

    const gp::FieldDescriptor *field_desc = nullptr;

    int arr_idx = 0;

private:
    enum class ParentType {
        MSG,
        ARR,
        MAP,
        SCALAR,
        INVALID
    };

    void _validate_parameters(gp::Message *parent_msg, const Path &path) const;

    ParentType _msg_field(const std::string &field, const gp::Reflection *reflection);

    ParentType _arr_field(const std::string &field, const gp::Reflection *reflection);
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_FIELD_REF_H
