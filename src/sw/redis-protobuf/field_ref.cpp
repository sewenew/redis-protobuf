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

#include "field_ref.h"
#include <google/protobuf/util/json_util.h>
#include "redis_protobuf.h"

namespace sw {

namespace redis {

namespace pb {

Path::Path(const StringView &str) {
    const auto *ptr = str.data();
    assert(ptr != nullptr);

    auto len = str.size();

    _type = _parse_type(ptr, len);

    if (_type.size() < len) {
        // Has fields.
        _fields = _parse_fields(ptr + _type.size(), len - _type.size());
    }
}

std::string Path::_parse_type(const char *ptr, std::size_t len) {
    assert(ptr != nullptr);

    if (len == 0) {
        throw Error("empty type");
    }

    std::size_t idx = 0;
    for (; idx != len; ++idx) {
        // e.g. type.field1.field2
        if (ptr[idx] == '.') {
            break;
        }
    }

    return std::string(ptr, idx);
}

std::vector<std::string> Path::_parse_fields(const char *ptr, std::size_t len) {
    assert(ptr != nullptr && len > 0 && ptr[0] == '.');

    std::vector<std::string> fields;
    std::size_t start = 1;
    for (std::size_t idx = 1; idx != len; ++idx) {
        if (ptr[idx] == '.') {
            if (idx <= start) {
                throw Error("empty field");
            }

            fields.emplace_back(ptr + start, idx - start);
            start = idx + 1;
        }
    }

    if (len <= start) {
        throw Error("empty field");
    }

    fields.emplace_back(ptr + start, len - start);

    return fields;
}

FieldRef::FieldRef(gp::Message *root_msg, const Path &path) {
    _validate_parameters(root_msg, path);

    msg = root_msg;

    auto parent_type = ParentType::MSG;

    for (const auto &field : path.fields()) {
        assert(!field.empty() && msg != nullptr);

        const auto *reflection = msg->GetReflection();

        if (field.back() == ']') {
            // It's an array or a map.
            parent_type = _aggregate_field(field, reflection);
            continue;
        }

        if (parent_type == ParentType::MSG) {
            parent_type = _msg_field(field, reflection);
        } else {
            throw Error("invalid path");
        }
    }
}

gp::FieldDescriptor::CppType FieldRef::type() const {
    if (field_desc == nullptr) {
        throw Error("no field specified");
    }

    return field_desc->cpp_type();
}

void FieldRef::_validate_parameters(gp::Message *root_msg, const Path &path) const {
    assert(root_msg != nullptr);

    if (root_msg->GetTypeName() != path.type()) {
        throw Error("type missmatch");
    }
}

FieldRef::ParentType FieldRef::_aggregate_field(const std::string &field,
        const gp::Reflection *reflection) {
    assert(!field.empty() && field.back() == ']');

    auto pos = field.find('[');
    if (pos == std::string::npos) {
        throw Error("invalid array or map");
    }

    auto name = field.substr(0, pos);
    auto key = field.substr(pos + 1, field.size() - pos - 2);

    field_desc = msg->GetDescriptor()->FindFieldByName(name);
    if (field_desc == nullptr) {
        throw Error("field not found: " + name);
    }

    if (field_desc->is_repeated()) {
        return _arr_field(key, reflection);
    } else if (field_desc->is_map()) {
        // TODO: support map
        assert(false);
    } else {
        throw Error("invalid array or map");
    }
}

FieldRef::ParentType FieldRef::_msg_field(const std::string &field,
        const gp::Reflection *reflection) {
    field_desc = msg->GetDescriptor()->FindFieldByName(field);
    if (field_desc == nullptr) {
        throw Error("field not found: " + field);
    }

    if (field_desc->is_repeated()) {
        return ParentType::ARR;
    } else if (field_desc->is_map()) {
        // TODO: how to do reflection with map?
        assert(false);
        return ParentType::MAP;
    } else if (type() == gp::FieldDescriptor::CPPTYPE_MESSAGE) {
        msg = reflection->MutableMessage(msg, field_desc);
        return ParentType::MSG;
    } else {
        return ParentType::SCALAR;
    }
}

FieldRef::ParentType FieldRef::_arr_field(const std::string &field,
        const gp::Reflection *reflection) {
    assert(field_desc != nullptr && msg != nullptr);

    arr_idx = -1;
    try {
        arr_idx = std::stoi(field);
    } catch (const std::exception &e) {
        throw Error("invalid array index: " + field);
    }

    auto size = reflection->FieldSize(*msg, field_desc);
    if (arr_idx >= size) {
        throw Error("array index is out-of-range: " + field + " : " + std::to_string(size));
    }

    switch (type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32:
    case gp::FieldDescriptor::CPPTYPE_INT64:
    case gp::FieldDescriptor::CPPTYPE_UINT32:
    case gp::FieldDescriptor::CPPTYPE_UINT64:
    case gp::FieldDescriptor::CPPTYPE_DOUBLE:
    case gp::FieldDescriptor::CPPTYPE_FLOAT:
    case gp::FieldDescriptor::CPPTYPE_BOOL:
    case gp::FieldDescriptor::CPPTYPE_STRING:
        return ParentType::SCALAR;

    case gp::FieldDescriptor::CPPTYPE_MESSAGE:
        msg = reflection->MutableRepeatedMessage(msg, field_desc, arr_idx);
        return ParentType::MSG;

    case gp::FieldDescriptor::CPPTYPE_ENUM:
        // TODO: support enum
        assert(false);

    // TODO: support map

    default:
        throw Error("invalid cpp type");
    }
}

}

}

}
