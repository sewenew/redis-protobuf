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
#include <google/protobuf/map_field.h>
#include <google/protobuf/map.h>
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
        if (_field_desc == nullptr) {
            throw Error("invalid path");
        }

        return _field_desc->cpp_type();
    }

    gp::FieldDescriptor::CppType map_value_type() const {
        const auto *val_desc = _mapped_value_desc();
        return val_desc->cpp_type();
    }

    std::string msg_type() const;

    std::string mapped_msg_type() const;

    bool is_array() const {
        return _field_desc != nullptr && _field_desc->is_repeated();
    }

    bool is_array_element() const {
        return _arr_idx >= 0;
    }

    bool is_map() const {
        return _field_desc != nullptr && _field_desc->is_map();
    }

    bool is_map_element() const {
        return bool(_map_key);
    }

    int size() const;

    FieldRef get_array_element(int idx) const;

    auto get_map_range() const ->
        std::pair<gp::Map<gp::MapKey, gp::MapValueRef>::const_iterator,
            gp::Map<gp::MapKey, gp::MapValueRef>::const_iterator>;

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

    int32_t get_mapped_int32() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetInt32Value();
    }

    int64_t get_mapped_int64() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetInt64Value();
    }

    uint32_t get_mapped_uint32() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetUInt32Value();
    }

    uint64_t get_mapped_uint64() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetUInt64Value();
    }

    float get_mapped_float() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetFloatValue();
    }

    double get_mapped_double() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetDoubleValue();
    }

    bool get_mapped_bool() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetBoolValue();
    }

    int get_mapped_enum() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetEnumValue();
    }

    std::string get_mapped_string() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetStringValue();
    }

    const gp::Message& get_mapped_msg() const {
        const auto &val = _get_map_value_const(_msg, _field_desc, *_map_key);
        return val.GetMessageValue();
    }

    void set_mapped_int32(int32_t val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        val_ref.SetInt32Value(val);
    }

    void set_mapped_int64(int64_t val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        val_ref.SetInt64Value(val);
    }

    void set_mapped_uint32(uint32_t val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        val_ref.SetUInt32Value(val);
    }

    void set_mapped_uint64(uint64_t val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        val_ref.SetUInt64Value(val);
    }

    void set_mapped_float(float val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        val_ref.SetFloatValue(val);
    }

    void set_mapped_double(double val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        val_ref.SetDoubleValue(val);
    }

    void set_mapped_bool(bool val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        val_ref.SetBoolValue(val);
    }

    void set_mapped_enum(int val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        val_ref.SetEnumValue(val);
    }

    void set_mapped_string(const std::string &val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        val_ref.SetStringValue(val);
    }

    void set_mapped_msg(const gp::Message &val) {
        auto &val_ref = _get_map_value(_msg, _field_desc, *_map_key);
        auto *msg = val_ref.MutableMessageValue();
        msg->CopyFrom(val);
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
    Msg* _get_sub_msg(Msg *msg, const gp::FieldDescriptor *field_desc, std::true_type) {
        return &(msg->GetReflection()->GetMessage(*msg, field_desc));
    }

    Msg* _get_sub_msg(Msg *msg, const gp::FieldDescriptor *field_desc, std::false_type) {
        return msg->GetReflection()->MutableMessage(msg, field_desc);
    }

    Msg* _get_sub_msg(Msg *msg, const gp::FieldDescriptor *field_desc) {
        assert(msg != nullptr);

        return _get_sub_msg(msg, field_desc, typename std::is_const<Msg>::type());
    }

    Msg* _get_sub_repeated_msg(Msg *msg,
            const gp::FieldDescriptor *field_desc,
            int idx,
            std::true_type) {
        return &(msg->GetReflection()->GetRepeatedMessage(*msg, field_desc, idx));
    }

    Msg* _get_sub_repeated_msg(Msg *msg,
            const gp::FieldDescriptor *field_desc,
            int idx,
            std::false_type) {
        return msg->GetReflection()->MutableRepeatedMessage(msg, field_desc, idx);
    }

    Msg* _get_sub_repeated_msg(Msg *msg,
            const gp::FieldDescriptor *field_desc,
            int idx) {
        assert(msg != nullptr);

        return _get_sub_repeated_msg(msg, field_desc, idx, typename std::is_const<Msg>::type());
    }

    class NotFoundError : public Error {
    public:
        NotFoundError() : Error("key not found") {}
    };

    const gp::MapValueRef& _get_map_value_const(Msg *msg,
            const gp::FieldDescriptor *field_desc,
            const gp::MapKey &key) const {
        // The following is hacking, hacking, and hacking!!!
        const auto *reflection =
            static_cast<const gp::internal::GeneratedMessageReflection*>(msg->GetReflection());
        const auto &map_base = reflection->GetRaw<gp::internal::MapFieldBase>(*msg, field_desc);
        const auto &dynamic_map = static_cast<const gp::internal::DynamicMapField&>(map_base);
        const auto &m = dynamic_map.GetMap();
        auto iter = m.find(key);
        if (iter == m.end()) {
            throw NotFoundError();
        }

        return iter->second;
    }

    gp::MapValueRef& _get_map_value(Msg *msg,
            const gp::FieldDescriptor *field_desc,
            const gp::MapKey &key) {
        // The following is hacking, hacking, and hacking!!!
        const auto *reflection =
            static_cast<const gp::internal::GeneratedMessageReflection*>(msg->GetReflection());
        auto *map_base = reflection->MutableRaw<gp::internal::MapFieldBase>(msg, field_desc);
        auto *dynamic_map = static_cast<gp::internal::DynamicMapField*>(map_base);

        // Ensure the value is initialized.
        gp::MapValueRef val;
        dynamic_map->InsertOrLookupMapValue(key, &val);

        auto *m = dynamic_map->MutableMap();
        auto iter = m->find(key);
        assert(iter != m->end());

        return iter->second;
    }

    Msg* _get_map_msg(Msg *msg,
            const gp::FieldDescriptor *field_desc,
            const gp::MapKey &key) {
        return _get_map_msg(msg, field_desc, key, typename std::is_const<Msg>::type());
    }

    Msg* _get_map_msg(Msg *msg,
            const gp::FieldDescriptor *field_desc,
            const gp::MapKey &key,
            std::true_type) {
        const auto &val = _get_map_value_const(msg, field_desc, key);
        const auto *val_desc = _mapped_value_desc();
        if (val_desc->cpp_type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
            throw Error("map value is not of message type");
        }

        return &val.GetMessageValue();
    }

    Msg* _get_map_msg(Msg *msg,
            const gp::FieldDescriptor *field_desc,
            const gp::MapKey &key,
            std::false_type) {
        auto &val = _get_map_value(msg, field_desc, key);
        auto *val_desc = _mapped_value_desc();
        if (val_desc->cpp_type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
            throw Error("map value is not of message type");
        }

        return val.MutableMessageValue();
    }

    const gp::FieldDescriptor* _mapped_value_desc() const {
        assert(is_map());

        auto *desc = _field_desc->message_type();
        assert(desc != nullptr);

        auto *val_desc = desc->FindFieldByName("value");
        assert(val_desc != nullptr);

        return val_desc;
    }

    void _validate_parameters(Msg *root_msg, const Path &path) const;

    void _parse_aggregate_field(const std::string &field);

    Optional<gp::MapKey> _parse_map_key(const std::string &key) {
        return _parse_map_key_impl(key, typename std::is_const<Msg>::type());
    }

    Optional<gp::MapKey> _parse_map_key_impl(const std::string &key);

    Optional<gp::MapKey> _parse_map_key_impl(const std::string &key, std::true_type);

    Optional<gp::MapKey> _parse_map_key_impl(const std::string &key, std::false_type) {
        return _parse_map_key_impl(key);
    }

    void _del_array_element();

    Msg *_msg = nullptr;

    const gp::FieldDescriptor *_field_desc = nullptr;

    int _arr_idx = -1;

    Optional<gp::MapKey> _map_key;
};

using ConstFieldRef = FieldRef<const gp::Message>;
using MutableFieldRef = FieldRef<gp::Message>;

template <typename Msg>
FieldRef<Msg>::FieldRef(Msg *root_msg, const Path &path) {
    _validate_parameters(root_msg, path);

    // Here we have to give _map_key a valid value.
    _map_key->SetBoolValue(false);

    _msg = root_msg;

    for (const auto &field : path.fields()) {
        assert(!field.empty() && _msg != nullptr);

        if (_field_desc != nullptr) {
            if (type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
                throw Error("invalid path");
            }

            if (_field_desc->is_map()) {
                if (!_map_key) {
                    throw Error("invalid path");
                }
                _msg = _get_map_msg(_msg, _field_desc, *_map_key);
                _map_key.reset();
                _map_key->SetBoolValue(false);
            } else if (_field_desc->is_repeated()) {
                if (_arr_idx < 0) {
                    throw Error("invalid path");
                }
                _msg = _get_sub_repeated_msg(_msg, _field_desc, _arr_idx);
                _arr_idx = -1;
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
FieldRef<Msg> FieldRef<Msg>::get_array_element(int idx) const {
    assert(is_array() && idx < size());

    FieldRef<Msg> element(*this);
    element._arr_idx = idx;

    return element;
}

template <typename Msg>
auto FieldRef<Msg>::get_map_range() const ->
    std::pair<gp::Map<gp::MapKey, gp::MapValueRef>::const_iterator,
        gp::Map<gp::MapKey, gp::MapValueRef>::const_iterator> {
    assert(is_map() && idx < size());

    // The following is hacking, hacking, and hacking!!!
    const auto *reflection =
        static_cast<const gp::internal::GeneratedMessageReflection*>(_msg->GetReflection());
    const auto &map_base = reflection->GetRaw<gp::internal::MapFieldBase>(*_msg, _field_desc);
    const auto &dynamic_map = static_cast<const gp::internal::DynamicMapField&>(map_base);
    const auto &m = dynamic_map.GetMap();

    return {m.begin(), m.end()};
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
std::string FieldRef<Msg>::mapped_msg_type() const {
    assert(_field_desc != nullptr);

    if (type() != gp::FieldDescriptor::CPPTYPE_MESSAGE) {
        throw Error("not a message");
    }

    auto *value_desc = _field_desc->message_type()->FindFieldByName("value");
    assert(value_desc != nullptr);

    return value_desc->message_type()->full_name();
}

template <typename Msg>
int FieldRef<Msg>::size() const {
    if (!(is_map() && !is_map_element()) && !(is_array() && !is_array_element())) {
        throw Error("not an array or map");
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

    if (is_map()) {
        _map_key = _parse_map_key(key);
    } else if (is_array()) {
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
        throw Error("not an array or map");
    }
}

template <typename Msg>
Optional<gp::MapKey> FieldRef<Msg>::_parse_map_key_impl(const std::string &key, std::true_type) {
    auto map_key = _parse_map_key_impl(key);

    try {
        _get_map_value_const(_msg, _field_desc, *map_key);
    } catch (const NotFoundError &e) {
        throw MapKeyNotFoundError(key);
    }

    return map_key;
}

template <typename Msg>
Optional<gp::MapKey> FieldRef<Msg>::_parse_map_key_impl(const std::string &key) {
    assert(is_map() && !_map_key);

    auto *desc = _field_desc->message_type();
    assert(desc != nullptr);

    auto *key_desc = desc->FindFieldByName("key");
    assert(key_desc != nullptr);

    // TODO: it seems we don't need a new map_key, just use _map_key
    gp::MapKey map_key;
    switch (key_desc->cpp_type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32:
        map_key.SetInt32Value(util::sv_to_int32(key));
        break;

    case gp::FieldDescriptor::CPPTYPE_INT64:
        map_key.SetInt64Value(util::sv_to_int64(key));
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT32:
        map_key.SetUInt32Value(util::sv_to_uint32(key));
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT64:
        map_key.SetUInt64Value(util::sv_to_uint64(key));
        break;

    case gp::FieldDescriptor::CPPTYPE_BOOL:
        map_key.SetBoolValue(util::sv_to_bool(key));
        break;

    case gp::FieldDescriptor::CPPTYPE_STRING:
        map_key.SetStringValue(key);
        break;

    default:
        // TODO: or assert?
        throw Error("invalid map key type");
    }

    return Optional<gp::MapKey>(map_key);
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
