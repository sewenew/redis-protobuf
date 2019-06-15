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

    for (const auto &field : path.fields()) {
        assert(!field.empty() && _msg != nullptr);

        const auto *reflection = _msg->GetReflection();

        if (_field_desc != nullptr){
            if (type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
                throw Error("invalid path");
            }

            if (_field_desc->is_repeated()) {
                assert(_arr_idx >= 0);
                _msg = reflection->MutableRepeatedMessage(_msg, _field_desc, _arr_idx);
                _arr_idx = -1;
            } else if (_field_desc->is_map()) {
                throw Error("map is not supported yet");
            } else {
                _msg = reflection->MutableMessage(_msg, _field_desc);
            }
        }

        if (field.back() == ']') {
            // It's an array or a map.
            _parse_aggregate_field(field);
        } else {
            _field_desc = _msg->GetDescriptor()->FindFieldByName(field);
            if (_field_desc == nullptr) {
                throw Error("field not found: " + field);
            }
        }
    }
}

gp::FieldDescriptor::CppType FieldRef::type() const {
    assert(_field_desc != nullptr);

    return _field_desc->cpp_type();
}

std::string FieldRef::msg_type() const {
    assert(_field_desc != nullptr);

    if (type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
        throw Error("not a message");
    }

    return _field_desc->message_type()->full_name();
}

int FieldRef::array_size() const {
    if (!is_array() || is_array_element()) {
        throw Error("not an array");
    }

    return _msg->GetReflection()->FieldSize(*_msg, _field_desc);
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

void FieldRef::set_enum(int val) {
    _msg->GetReflection()->SetEnumValue(_msg, _field_desc, val);
}

void FieldRef::set_string(const std::string &val) {
    _msg->GetReflection()->SetString(_msg, _field_desc, val);
}

void FieldRef::set_msg(gp::Message &msg) {
    auto sub_msg = _msg->GetReflection()->MutableMessage(_msg, _field_desc);

    assert(sub_msg->GetTypeName() == msg.GetTypeName());

    sub_msg->GetReflection()->Swap(sub_msg, &msg);
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

int FieldRef::get_enum() const {
    return _msg->GetReflection()->GetEnumValue(*_msg, _field_desc);
}

std::string FieldRef::get_string() const {
    return _msg->GetReflection()->GetString(*_msg, _field_desc);
}

const gp::Message& FieldRef::get_msg() const {
    return _msg->GetReflection()->GetMessage(*_msg, _field_desc);
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

void FieldRef::set_repeated_enum(int val) {
    _msg->GetReflection()->SetRepeatedEnumValue(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_string(const std::string &val) {
    _msg->GetReflection()->SetRepeatedString(_msg, _field_desc, _arr_idx, val);
}

void FieldRef::set_repeated_msg(gp::Message &msg) {
    auto *sub_msg = _msg->GetReflection()->MutableRepeatedMessage(_msg, _field_desc, _arr_idx);
    sub_msg->GetReflection()->Swap(sub_msg, &msg);
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

int FieldRef::get_repeated_enum() const {
    return _msg->GetReflection()->GetRepeatedEnumValue(*_msg, _field_desc, _arr_idx);
}

std::string FieldRef::get_repeated_string() const {
    return _msg->GetReflection()->GetRepeatedString(*_msg, _field_desc, _arr_idx);
}

const gp::Message& FieldRef::get_repeated_msg() const {
    return _msg->GetReflection()->GetRepeatedMessage(*_msg, _field_desc, _arr_idx);
}

void FieldRef::add_int32(int32_t val) {
    _msg->GetReflection()->AddInt32(_msg, _field_desc, val);
}

void FieldRef::add_int64(int64_t val) {
    _msg->GetReflection()->AddInt64(_msg, _field_desc, val);
}

void FieldRef::add_uint32(uint32_t val) {
    _msg->GetReflection()->AddUInt32(_msg, _field_desc, val);
}

void FieldRef::add_uint64(uint64_t val) {
    _msg->GetReflection()->AddUInt64(_msg, _field_desc, val);
}

void FieldRef::add_float(float val) {
    _msg->GetReflection()->AddFloat(_msg, _field_desc, val);
}

void FieldRef::add_double(double val) {
    _msg->GetReflection()->AddDouble(_msg, _field_desc, val);
}

void FieldRef::add_bool(bool val) {
    _msg->GetReflection()->AddBool(_msg, _field_desc, val);
}

void FieldRef::add_enum(int val) {
    _msg->GetReflection()->AddEnumValue(_msg, _field_desc, val);
}

void FieldRef::add_string(const std::string &val) {
    _msg->GetReflection()->AddString(_msg, _field_desc, val);
}

void FieldRef::add_msg(gp::Message &msg) {
    auto sub_msg = _msg->GetReflection()->AddMessage(_msg, _field_desc);

    assert(sub_msg->GetTypeName() == msg.GetTypeName());

    sub_msg->GetReflection()->Swap(sub_msg, &msg);
}

void FieldRef::clear() {
    if (is_array_element()) {
        throw Error("cannot clear an array element");
    }

    // TODO: map element.

    if (_field_desc == nullptr) {
        _msg->Clear();
    } else {
        _msg->GetReflection()->ClearField(_msg, _field_desc);
    }
}

void FieldRef::del() {
    if (is_array_element()) {
        _del_array_element();
    } else {
        // TODO: support map
        throw Error("can only delete array element");
    }
}

void FieldRef::merge(const gp::Message &msg) {
    assert(_field_desc != nullptr);

    if (type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
        throw Error("not a message");
    }

    auto sub_msg = _msg->GetReflection()->MutableMessage(_msg, _field_desc);

    assert(sub_msg->GetTypeName() == msg.GetTypeName());

    sub_msg->MergeFrom(msg);
}

void FieldRef::_validate_parameters(gp::Message *root_msg, const Path &path) const {
    assert(root_msg != nullptr);

    if (root_msg->GetTypeName() != path.type()) {
        throw Error("type missmatch");
    }
}

void FieldRef::_parse_aggregate_field(const std::string &field) {
    assert(!field.empty() && field.back() == ']');

    auto pos = field.find('[');
    if (pos == std::string::npos) {
        throw Error("invalid array or map");
    }

    auto name = field.substr(0, pos);
    auto key = field.substr(pos + 1, field.size() - pos - 2);

    _field_desc = _msg->GetDescriptor()->FindFieldByName(name);

    if (_field_desc->is_repeated()) {
        try {
            _arr_idx = std::stoi(key);
        } catch (const std::exception &e) {
            throw Error("invalid array index: " + key);
        }

        auto size = _msg->GetReflection()->FieldSize(*_msg, _field_desc);
        if (_arr_idx >= size) {
            throw Error("array index is out-of-range: " + key + " : " + std::to_string(size));
        }
    } else {
        // TODO: support map
        throw Error("not an array or map");
    }
}

void FieldRef::_del_array_element() {
    assert(is_array_element());

    const auto *reflection = _msg->GetReflection();
    auto size = reflection->FieldSize(*_msg, _field_desc);
    for (auto idx = _arr_idx; idx != size - 1; ++idx) {
        reflection->SwapElements(_msg, _field_desc, idx, idx + 1);
    }

    reflection->RemoveLast(_msg, _field_desc);
}

}

}

}
