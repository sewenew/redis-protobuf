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

    _msg = root_msg;

    auto parent_type = ParentType::MSG;

    for (const auto &field : path.fields()) {
        assert(!field.empty() && _msg != nullptr);

        const auto *reflection = _msg->GetReflection();

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
    if (_field_desc == nullptr) {
        throw Error("no field specified");
    }

    return _field_desc->cpp_type();
}

void FieldRef::set_int32(int32_t val) {
    _msg->GetReflection()->SetInt32(_msg, _field_desc, val);
}

void FieldRef::set_int64(int64_t val) {
    _msg->GetReflection()->SetInt64(_msg, _field_desc, val);
}

void FieldRef::set_uint32(uint32_t val) {
    _msg->GetReflection()->SetUInt32(_msg, _field_desc, val);
}

void FieldRef::set_uint64(uint64_t val) {
    _msg->GetReflection()->SetUInt64(_msg, _field_desc, val);
}

void FieldRef::set_float(float val) {
    _msg->GetReflection()->SetFloat(_msg, _field_desc, val);
}

void FieldRef::set_double(double val) {
    _msg->GetReflection()->SetDouble(_msg, _field_desc, val);
}

void FieldRef::set_bool(bool val) {
    _msg->GetReflection()->SetBool(_msg, _field_desc, val);
}

void FieldRef::set_string(const std::string &val) {
    _msg->GetReflection()->SetString(_msg, _field_desc, val);
}

void FieldRef::set_msg(gp::Message &msg) {
    _msg->GetReflection()->Swap(_msg, &msg);
}

int32_t FieldRef::get_int32() const {
    return _msg->GetReflection()->GetInt32(*_msg, _field_desc);
}

int64_t FieldRef::get_int64() const {
    return _msg->GetReflection()->GetInt64(*_msg, _field_desc);
}

uint32_t FieldRef::get_uint32() const {
    return _msg->GetReflection()->GetUInt32(*_msg, _field_desc);
}

uint64_t FieldRef::get_uint64() const {
    return _msg->GetReflection()->GetUInt64(*_msg, _field_desc);
}

float FieldRef::get_float() const {
    return _msg->GetReflection()->GetFloat(*_msg, _field_desc);
}

double FieldRef::get_double() const {
    return _msg->GetReflection()->GetDouble(*_msg, _field_desc);
}

bool FieldRef::get_bool() const {
    return _msg->GetReflection()->GetBool(*_msg, _field_desc);
}

std::string FieldRef::get_string() const {
    return _msg->GetReflection()->GetString(*_msg, _field_desc);
}

void FieldRef::set_repeated_int32(int32_t val) {
    _msg->GetReflection()->SetRepeatedInt32(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_int64(int64_t val) {
    _msg->GetReflection()->SetRepeatedInt64(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_uint32(uint32_t val) {
    _msg->GetReflection()->SetRepeatedUInt32(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_uint64(uint64_t val) {
    _msg->GetReflection()->SetRepeatedUInt64(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_float(float val) {
    _msg->GetReflection()->SetRepeatedFloat(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_double(double val) {
    _msg->GetReflection()->SetRepeatedDouble(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_bool(bool val) {
    _msg->GetReflection()->SetRepeatedBool(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_string(const std::string &val) {
    _msg->GetReflection()->SetRepeatedString(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_msg(gp::Message &msg) {
    const auto *reflection = _msg->GetReflection();
    auto *sub_msg = reflection->MutableRepeatedMessage(_msg, _field_desc, _arr_idx);
    reflection->Swap(sub_msg, &msg);
}

int32_t FieldRef::get_repeated_int32() const {
    return _msg->GetReflection()->GetRepeatedInt32(*_msg, _field_desc, _arr_idx);
}

int64_t FieldRef::get_repeated_int64() const {
    return _msg->GetReflection()->GetRepeatedInt64(*_msg, _field_desc, _arr_idx);
}

uint32_t FieldRef::get_repeated_uint32() const {
    return _msg->GetReflection()->GetRepeatedUInt32(*_msg, _field_desc, _arr_idx);
}

uint64_t FieldRef::get_repeated_uint64() const {
    return _msg->GetReflection()->GetRepeatedUInt64(*_msg, _field_desc, _arr_idx);
}

float FieldRef::get_repeated_float() const {
    return _msg->GetReflection()->GetRepeatedFloat(*_msg, _field_desc, _arr_idx);
}

double FieldRef::get_repeated_double() const {
    return _msg->GetReflection()->GetRepeatedDouble(*_msg, _field_desc, _arr_idx);
}

bool FieldRef::get_repeated_bool() const {
    return _msg->GetReflection()->GetRepeatedBool(*_msg, _field_desc, _arr_idx);
}

std::string FieldRef::get_repeated_string() const {
    return _msg->GetReflection()->GetRepeatedString(*_msg, _field_desc, _arr_idx);
}

const gp::Message& FieldRef::get_repeated_msg() const {
    return _msg->GetReflection()->GetRepeatedMessage(*_msg, _field_desc, _arr_idx);
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

    _field_desc = _msg->GetDescriptor()->FindFieldByName(name);
    if (_field_desc == nullptr) {
        throw Error("field not found: " + name);
    }

    if (_field_desc->is_repeated()) {
        return _arr_field(key, reflection);
    } else if (_field_desc->is_map()) {
        // TODO: support map
        assert(false);
    } else {
        throw Error("invalid array or map");
    }
}

FieldRef::ParentType FieldRef::_msg_field(const std::string &field,
        const gp::Reflection *reflection) {
    _field_desc = _msg->GetDescriptor()->FindFieldByName(field);
    if (_field_desc == nullptr) {
        throw Error("field not found: " + field);
    }

    if (_field_desc->is_repeated()) {
        return ParentType::ARR;
    } else if (_field_desc->is_map()) {
        // TODO: how to do reflection with map?
        assert(false);
        return ParentType::MAP;
    } else if (type() == gp::FieldDescriptor::CPPTYPE_MESSAGE) {
        _msg = reflection->MutableMessage(_msg, _field_desc);
        return ParentType::MSG;
    } else {
        return ParentType::SCALAR;
    }
}

FieldRef::ParentType FieldRef::_arr_field(const std::string &field,
        const gp::Reflection *reflection) {
    assert(_field_desc != nullptr && _msg != nullptr);

    _arr_idx = -1;
    try {
        _arr_idx = std::stoi(field);
    } catch (const std::exception &e) {
        throw Error("invalid array index: " + field);
    }

    auto size = reflection->FieldSize(*_msg, _field_desc);
    if (_arr_idx >= size) {
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
        _msg = reflection->MutableRepeatedMessage(_msg, _field_desc, _arr_idx);
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
