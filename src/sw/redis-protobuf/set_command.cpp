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

#include "set_command.h"
#include "errors.h"
#include "redis_protobuf.h"
#include "utils.h"
#include "field_ref.h"

namespace sw {

namespace redis {

namespace pb {

int SetCommand::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        assert(ctx != nullptr);

        // TODO: if key exist and type mismatch, return false
        // TODO: add NX, XX and EX support
        // TODO: if the ByteSize is too large, serialization might fail.

        auto args = _parse_args(argv, argc);
        const auto &path = args.path;

        auto key = api::open_key(ctx, args.key_name, api::KeyMode::WRITEONLY);
        assert(key);

        if (!api::key_exists(key.get(), RedisProtobuf::instance().type())) {
            _create_msg(*key, path, args.val);
        } else {
            _set_msg(*key, path, args.val);
        }

        return RedisModule_ReplyWithLongLong(ctx, 1);
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

SetCommand::Args SetCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc != 4) {
        throw WrongArityError();
    }

    return {argv[1], Path(argv[2]), StringView(argv[3])};
}

void SetCommand::_create_msg(RedisModuleKey &key,
        const Path &path,
        const StringView &val) const {
    MsgUPtr msg;
    auto &module = RedisProtobuf::instance();
    if (path.empty()) {
        msg = module.proto_factory()->create(path.type(), val);
    } else {
        msg = module.proto_factory()->create(path.type());
        FieldRef field(msg.get(), path);
        _set_field(field, val);
    }

    if (RedisModule_ModuleTypeSetValue(&key, module.type(), msg.get()) != REDISMODULE_OK) {
        throw Error("failed to set message");
    }

    msg.release();
}

void SetCommand::_set_msg(RedisModuleKey &key,
        const Path &path,
        const StringView &val) const {
    if (path.empty()) {
        // Set the whole message.
        auto &module = RedisProtobuf::instance();
        auto msg = module.proto_factory()->create(path.type(), val);
        if (RedisModule_ModuleTypeSetValue(&key, module.type(), msg.get()) != REDISMODULE_OK) {
            throw Error("failed to set message");
        }

        msg.release();
    } else {
        // Set field.
        auto *msg = api::get_msg_by_key(&key);
        assert(msg != nullptr);

        FieldRef field(msg, path);
        _set_field(field, val);
    }
}

void SetCommand::_set_field(FieldRef &field, const StringView &val) const {
    if (field.is_array_element()) {
        return _set_array_element(field, val);
    } else if (field.is_array()) {
        throw Error("cannot set the whole array field");
    }

    if (field.field_desc->is_map()) {
        throw Error("cannot set map field");
    }

    _set_scalar_field(field, val);
}

void SetCommand::_set_scalar_field(FieldRef &field, const StringView &val) const {
    switch (field.type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32:
        _set_int32(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_INT64:
        _set_int64(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT32:
        _set_uint32(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT64:
        _set_uint64(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_DOUBLE:
        _set_double(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_FLOAT:
        _set_float(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_BOOL:
        _set_bool(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_STRING:
        _set_string(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_MESSAGE:
        _set_msg(field, val);
        break;

    default:
        throw Error("unknown type");
    }
}

void SetCommand::_set_array_element(FieldRef &field, const StringView &val) const {
    switch (field.type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32:
        _set_repeated_int32(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_INT64:
        _set_repeated_int64(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT32:
        _set_repeated_uint32(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT64:
        _set_repeated_uint64(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_DOUBLE:
        _set_repeated_double(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_FLOAT:
        _set_repeated_float(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_BOOL:
        _set_repeated_bool(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_STRING:
        _set_repeated_string(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_MESSAGE:
        _set_repeated_msg(field, val);
        break;

    default:
        throw Error("unknown type");
    }
}

void SetCommand::_set_int32(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_INT32);

    try {
        auto val = std::stoi(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetInt32(field.msg, field.field_desc, val);
    } catch (const std::exception &e) {
        throw Error("not int32");
    }
}

void SetCommand::_set_int64(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_INT64);

    try {
        auto val = std::stoll(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetInt64(field.msg, field.field_desc, val);
    } catch (const std::exception &e) {
        throw Error("not int64");
    }
}

void SetCommand::_set_uint32(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_UINT32);

    try {
        auto val = std::stoul(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetUInt32(field.msg, field.field_desc, val);
    } catch (const std::exception &e) {
        throw Error("not uint32");
    }
}

void SetCommand::_set_uint64(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_UINT64);

    try {
        auto val = std::stoull(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetUInt64(field.msg, field.field_desc, val);
    } catch (const std::exception &e) {
        throw Error("not uint64");
    }
}

void SetCommand::_set_double(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_DOUBLE);

    try {
        auto val = std::stod(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetDouble(field.msg, field.field_desc, val);
    } catch (const std::exception &e) {
        throw Error("not double");
    }
}

void SetCommand::_set_float(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_FLOAT);

    try {
        auto val = std::stof(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetFloat(field.msg, field.field_desc, val);
    } catch (const std::exception &e) {
        throw Error("not float");
    }
}

void SetCommand::_set_bool(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_BOOL);

    bool b = false;
    auto s = std::string(sv.data(), sv.size());
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

    field.msg->GetReflection()->SetBool(field.msg, field.field_desc, b);
}

void SetCommand::_set_string(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_STRING);

    field.msg->GetReflection()->SetString(field.msg,
            field.field_desc,
            std::string(sv.data(), sv.size()));
}

void SetCommand::_set_msg(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_MESSAGE);

    auto new_msg = RedisProtobuf::instance().proto_factory()->create(field.msg->GetTypeName(), sv);

    field.msg->GetReflection()->Swap(field.msg, new_msg.get());
}

void SetCommand::_set_repeated_int32(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_INT32);

    try {
        auto val = std::stoi(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetRepeatedInt32(field.msg,
                field.field_desc,
                field.arr_idx,
                val);
    } catch (const std::exception &e) {
        throw Error("not int32");
    }
}

void SetCommand::_set_repeated_int64(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_INT64);

    try {
        auto val = std::stoll(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetRepeatedInt64(field.msg,
                field.field_desc,
                field.arr_idx,
                val);
    } catch (const std::exception &e) {
        throw Error("not int64");
    }
}

void SetCommand::_set_repeated_uint32(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_UINT32);

    try {
        auto val = std::stoul(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetRepeatedUInt32(field.msg,
                field.field_desc,
                field.arr_idx,
                val);
    } catch (const std::exception &e) {
        throw Error("not uint32");
    }
}

void SetCommand::_set_repeated_uint64(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_UINT64);

    try {
        auto val = std::stoull(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetRepeatedUInt64(field.msg,
                field.field_desc,
                field.arr_idx,
                val);
    } catch (const std::exception &e) {
        throw Error("not uint64");
    }
}

void SetCommand::_set_repeated_double(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_DOUBLE);

    try {
        auto val = std::stod(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetRepeatedDouble(field.msg,
                field.field_desc,
                field.arr_idx,
                val);
    } catch (const std::exception &e) {
        throw Error("not double");
    }
}

void SetCommand::_set_repeated_float(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_FLOAT);

    try {
        auto val = std::stof(std::string(sv.data(), sv.size()));
        field.msg->GetReflection()->SetRepeatedFloat(field.msg,
                field.field_desc,
                field.arr_idx,
                val);
    } catch (const std::exception &e) {
        throw Error("not float");
    }
}

void SetCommand::_set_repeated_bool(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_BOOL);

    bool b = false;
    auto s = std::string(sv.data(), sv.size());
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

    field.msg->GetReflection()->SetRepeatedBool(field.msg,
            field.field_desc,
            field.arr_idx,
            b);
}

void SetCommand::_set_repeated_string(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_STRING);

    field.msg->GetReflection()->SetRepeatedString(field.msg,
            field.field_desc,
            field.arr_idx,
            std::string(sv.data(), sv.size()));
}

void SetCommand::_set_repeated_msg(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_MESSAGE);

    auto new_msg = RedisProtobuf::instance().proto_factory()->create(field.msg->GetTypeName(), sv);

    const auto *reflection = field.msg->GetReflection();
    auto *msg = reflection->MutableRepeatedMessage(field.msg, field.field_desc, field.arr_idx);
    reflection->Swap(msg, new_msg.get());
}

}

}

}
