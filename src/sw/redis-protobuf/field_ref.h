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

#include <cassert>
#include <string>
#include <type_traits>
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

    Path() = default;

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
    std::pair<std::string, std::size_t> _parse_type(const char *ptr, std::size_t len);

    std::vector<std::string> _parse_fields(const char *ptr, std::size_t len);

    std::string _type;

    std::vector<std::string> _fields;
};

template <typename Msg>
class FieldRef {
public:
    FieldRef(Msg *root_msg, const Path &path);

    gp::FieldDescriptor::CppType type() const {
        assert(_field_desc != nullptr);
        return _field_desc->cpp_type();
    }

    std::string msg_type() const;

    bool is_array() const {
        return _field_desc != nullptr && _field_desc->is_repeated();
    }

    bool is_array_element() const {
        return _arr_idx >= 0;
    }

    bool is_map() const {
        return _field_desc != nullptr && _field_desc->is_map();
    }

    int array_size() const;

    explicit operator bool() const {
        return _field_desc != nullptr;
    }

    int32_t get_int32() const {
        return _msg->GetReflection()->GetInt32(*_msg, _field_desc);
    }

    int64_t get_int64() const {
        return _msg->GetReflection()->GetInt64(*_msg, _field_desc);
    }

    uint32_t get_uint32() const {
        return _msg->GetReflection()->GetUInt32(*_msg, _field_desc);
    }

    uint64_t get_uint64() const {
        return _msg->GetReflection()->GetUInt64(*_msg, _field_desc);
    }

    float get_float() const {
        return _msg->GetReflection()->GetFloat(*_msg, _field_desc);
    }

    double get_double() const {
        return _msg->GetReflection()->GetDouble(*_msg, _field_desc);
    }

    bool get_bool() const {
        return _msg->GetReflection()->GetBool(*_msg, _field_desc);
    }

    int get_enum() const {
        return _msg->GetReflection()->GetEnumValue(*_msg, _field_desc);
    }

    std::string get_string() const {
        return _msg->GetReflection()->GetString(*_msg, _field_desc);
    }

    const gp::Message& get_msg() const {
        return _msg->GetReflection()->GetMessage(*_msg, _field_desc);
    }

    int32_t get_repeated_int32() const {
        return _msg->GetReflection()->GetRepeatedInt32(*_msg, _field_desc, _arr_idx);
    }

    int64_t get_repeated_int64() const {
        return _msg->GetReflection()->GetRepeatedInt64(*_msg, _field_desc, _arr_idx);
    }

    uint32_t get_repeated_uint32() const {
        return _msg->GetReflection()->GetRepeatedUInt32(*_msg, _field_desc, _arr_idx);
    }

    uint64_t get_repeated_uint64() const {
        return _msg->GetReflection()->GetRepeatedUInt64(*_msg, _field_desc, _arr_idx);
    }

    float get_repeated_float() const {
        return _msg->GetReflection()->GetRepeatedFloat(*_msg, _field_desc, _arr_idx);
    }

    double get_repeated_double() const {
        return _msg->GetReflection()->GetRepeatedDouble(*_msg, _field_desc, _arr_idx);
    }

    bool get_repeated_bool() const {
        return _msg->GetReflection()->GetRepeatedBool(*_msg, _field_desc, _arr_idx);
    }

    int get_repeated_enum() const {
        return _msg->GetReflection()->GetRepeatedEnumValue(*_msg, _field_desc, _arr_idx);
    }

    std::string get_repeated_string() const {
        return _msg->GetReflection()->GetRepeatedString(*_msg, _field_desc, _arr_idx);
    }

    const gp::Message& get_repeated_msg() const {
        return _msg->GetReflection()->GetRepeatedMessage(*_msg, _field_desc, _arr_idx);
    }

    void set_int32(int32_t val);
    void set_int64(int64_t val);
    void set_uint32(uint32_t val);
    void set_uint64(uint64_t val);
    void set_float(float val);
    void set_double(double val);
    void set_bool(bool val);
    void set_enum(int val);
    void set_string(const std::string &val);
    void set_msg(gp::Message &msg);

    void set_repeated_int32(int32_t val);
    void set_repeated_int64(int64_t val);
    void set_repeated_uint32(uint32_t val);
    void set_repeated_uint64(uint64_t val);
    void set_repeated_float(float val);
    void set_repeated_double(double val);
    void set_repeated_bool(bool val);
    void set_repeated_enum(int val);
    void set_repeated_string(const std::string &val);
    void set_repeated_msg(gp::Message &msg);

    void add_int32(int32_t val);
    void add_int64(int64_t val);
    void add_uint32(uint32_t val);
    void add_uint64(uint64_t val);
    void add_float(float val);
    void add_double(double val);
    void add_bool(bool val);
    void add_enum(int val);
    void add_string(const std::string &val);
    void add_msg(gp::Message &msg);

    void clear();

    void del();

    void merge(const gp::Message &msg);

private:
    template <typename T>
    T* _get_sub_msg(T *msg, const gp::FieldDescriptor *field_desc, std::true_type) {
        return &(msg->GetReflection()->GetMessage(*msg, field_desc));
    }

    template <typename T>
    T* _get_sub_msg(T *msg, const gp::FieldDescriptor *field_desc, std::false_type) {
        return msg->GetReflection()->MutableMessage(msg, field_desc);
    }

    template <typename T>
    T* _get_sub_msg(T *msg, const gp::FieldDescriptor *field_desc) {
        assert(msg != nullptr);

        return _get_sub_msg(msg, field_desc, typename std::is_const<Msg>::type());
    }

    template <typename T>
    T* _get_sub_repeated_msg(T *msg,
            const gp::FieldDescriptor *field_desc,
        int idx,
        std::true_type) {
        return &(msg->GetReflection()->GetRepeatedMessage(*msg, field_desc, idx));
    }

    template <typename T>
    T* _get_sub_repeated_msg(T *msg,
            const gp::FieldDescriptor *field_desc,
            int idx,
            std::false_type) {
        return msg->GetReflection()->MutableRepeatedMessage(msg, field_desc, idx);
    }

    template <typename T>
    T* _get_sub_repeated_msg(T *msg,
            const gp::FieldDescriptor *field_desc,
            int idx) {
        assert(msg != nullptr);

        return _get_sub_repeated_msg(msg, field_desc, idx, typename std::is_const<Msg>::type());
    }

    Msg *_msg = nullptr;

    const gp::FieldDescriptor *_field_desc = nullptr;

    int _arr_idx = -1;

    void _validate_parameters(Msg *root_msg, const Path &path) const;

    void _parse_aggregate_field(const std::string &field);

    void _del_array_element();
};

using ConstFieldRef = FieldRef<const gp::Message>;
using MutableFieldRef = FieldRef<gp::Message>;

template <typename Msg>
FieldRef<Msg>::FieldRef(Msg *root_msg, const Path &path) {
    _validate_parameters(root_msg, path);

    _msg = root_msg;

    for (const auto &field : path.fields()) {
        assert(!field.empty() && _msg != nullptr);

        if (_field_desc != nullptr){
            if (type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
                throw Error("invalid path");
            }

            if (_field_desc->is_repeated()) {
                assert(_arr_idx >= 0);
                _msg = _get_sub_repeated_msg(_msg, _field_desc, _arr_idx);
                _arr_idx = -1;
            } else if (_field_desc->is_map()) {
                throw Error("map is not supported yet");
            } else {
                _msg = _get_sub_msg(_msg, _field_desc);
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

template <typename Msg>
std::string FieldRef<Msg>::msg_type() const {
    assert(_field_desc != nullptr);

    if (type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
        throw Error("not a message");
    }

    return _field_desc->message_type()->full_name();
}

template <typename Msg>
int FieldRef<Msg>::array_size() const {
    if (!is_array() || is_array_element()) {
        throw Error("not an array");
    }

    return _msg->GetReflection()->FieldSize(*_msg, _field_desc);
}

template <typename Msg>
void FieldRef<Msg>::_validate_parameters(Msg *root_msg, const Path &path) const {
    assert(root_msg != nullptr);

    if (root_msg->GetTypeName() != path.type()) {
        throw Error("type missmatch");
    }
}

template <typename Msg>
void FieldRef<Msg>::_parse_aggregate_field(const std::string &field) {
    assert(!field.empty() && field.back() == ']');

    auto pos = field.find('[');
    if (pos == std::string::npos) {
        throw Error("invalid array or map");
    }

    auto name = field.substr(0, pos);
    auto key = field.substr(pos + 1, field.size() - pos - 2);

    _field_desc = _msg->GetDescriptor()->FindFieldByName(name);
    if (_field_desc == nullptr) {
        throw Error("invalid field: " + name);
    }

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

template <typename Msg>
void FieldRef<Msg>::set_int32(int32_t val) {
    _msg->GetReflection()->SetInt32(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::set_int64(int64_t val) {
    _msg->GetReflection()->SetInt64(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::set_uint32(uint32_t val) {
    _msg->GetReflection()->SetUInt32(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::set_uint64(uint64_t val) {
    _msg->GetReflection()->SetUInt64(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::set_float(float val) {
    _msg->GetReflection()->SetFloat(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::set_double(double val) {
    _msg->GetReflection()->SetDouble(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::set_bool(bool val) {
    _msg->GetReflection()->SetBool(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::set_enum(int val) {
    _msg->GetReflection()->SetEnumValue(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::set_string(const std::string &val) {
    _msg->GetReflection()->SetString(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::set_msg(gp::Message &msg) {
    auto sub_msg = _msg->GetReflection()->MutableMessage(_msg, _field_desc);

    assert(sub_msg->GetTypeName() == msg.GetTypeName());

    sub_msg->GetReflection()->Swap(sub_msg, &msg);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_int32(int32_t val) {
    _msg->GetReflection()->SetRepeatedInt32(_msg, _field_desc, _arr_idx, val);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_int64(int64_t val) {
    _msg->GetReflection()->SetRepeatedInt64(_msg, _field_desc, _arr_idx, val);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_uint32(uint32_t val) {
    _msg->GetReflection()->SetRepeatedUInt32(_msg, _field_desc, _arr_idx, val);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_uint64(uint64_t val) {
    _msg->GetReflection()->SetRepeatedUInt64(_msg, _field_desc, _arr_idx, val);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_float(float val) {
    _msg->GetReflection()->SetRepeatedFloat(_msg, _field_desc, _arr_idx, val);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_double(double val) {
    _msg->GetReflection()->SetRepeatedDouble(_msg, _field_desc, _arr_idx, val);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_bool(bool val) {
    _msg->GetReflection()->SetRepeatedBool(_msg, _field_desc, _arr_idx, val);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_enum(int val) {
    _msg->GetReflection()->SetRepeatedEnumValue(_msg, _field_desc, _arr_idx, val);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_string(const std::string &val) {
    _msg->GetReflection()->SetRepeatedString(_msg, _field_desc, _arr_idx, val);
}

template <typename Msg>
void FieldRef<Msg>::set_repeated_msg(gp::Message &msg) {
    auto *sub_msg = _msg->GetReflection()->MutableRepeatedMessage(_msg, _field_desc, _arr_idx);
    sub_msg->GetReflection()->Swap(sub_msg, &msg);
}

template <typename Msg>
void FieldRef<Msg>::add_int32(int32_t val) {
    _msg->GetReflection()->AddInt32(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::add_int64(int64_t val) {
    _msg->GetReflection()->AddInt64(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::add_uint32(uint32_t val) {
    _msg->GetReflection()->AddUInt32(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::add_uint64(uint64_t val) {
    _msg->GetReflection()->AddUInt64(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::add_float(float val) {
    _msg->GetReflection()->AddFloat(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::add_double(double val) {
    _msg->GetReflection()->AddDouble(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::add_bool(bool val) {
    _msg->GetReflection()->AddBool(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::add_enum(int val) {
    _msg->GetReflection()->AddEnumValue(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::add_string(const std::string &val) {
    _msg->GetReflection()->AddString(_msg, _field_desc, val);
}

template <typename Msg>
void FieldRef<Msg>::add_msg(gp::Message &msg) {
    auto sub_msg = _msg->GetReflection()->AddMessage(_msg, _field_desc);

    assert(sub_msg->GetTypeName() == msg.GetTypeName());

    sub_msg->GetReflection()->Swap(sub_msg, &msg);
}

template <typename Msg>
void FieldRef<Msg>::clear() {
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

template <typename Msg>
void FieldRef<Msg>::del() {
    if (is_array_element()) {
        _del_array_element();
    } else {
        // TODO: support map
        throw Error("can only delete array element");
    }
}

template <typename Msg>
void FieldRef<Msg>::merge(const gp::Message &msg) {
    assert(_field_desc != nullptr);

    if (type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
        throw Error("not a message");
    }

    auto sub_msg = _msg->GetReflection()->MutableMessage(_msg, _field_desc);

    assert(sub_msg->GetTypeName() == msg.GetTypeName());

    sub_msg->MergeFrom(msg);
}

template <typename Msg>
void FieldRef<Msg>::_del_array_element() {
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

#endif // end SEWENEW_REDISPROTOBUF_FIELD_REF_H
