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

        auto msg = _build_msg(args.paths);
        assert(msg);

        _set_msg(ctx, args.key_name, std::move(msg));

        return RedisModule_ReplyWithLongLong(ctx, 1);
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }
}

SetCommand::Args SetCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 4 || argc % 2 != 0) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];
    args.paths.reserve((argc - 2) / 2);

    std::string type;
    for (auto idx = 2; idx != argc; idx += 2) {
        Path path(argv[idx]);
        if (type.empty()) {
            type = path.type();
        } else {
            if (type != path.type()) {
                throw Error("fields have different types");
            }
        }

        args.paths.emplace_back(std::move(path), StringView(argv[idx + 1]));
    }

    return args;
}

MsgUPtr SetCommand::_build_msg(const std::vector<std::pair<Path, StringView>> &paths) const {
    assert(paths.size() > 0);

    const auto &first_path = paths[0].first;
    const auto &type = first_path.type();

    auto &module = RedisProtobuf::instance();

    MsgUPtr msg;
    std::size_t idx = 0;
    if (first_path.empty()) {
        // Create a message with the given data.
        msg = module.proto_factory()->create(type, paths[0].second);
        idx = 1;
    } else {
        // Create an empty message.
        msg = module.proto_factory()->create(type);
    }

    assert(msg);

    for (; idx != paths.size(); ++idx) {
        const auto &path = paths[idx].first;
        const auto &val = paths[idx].second;
        if (path.empty()) {
            _set_msg(msg.get(), type, val);
        } else {
            _set_field(msg.get(), path, val);
        }
    }

    return msg;
}

void SetCommand::_set_msg(RedisModuleCtx *ctx, RedisModuleString *key_name, MsgUPtr msg) const {
    assert(ctx != nullptr && key_name != nullptr && msg);

    auto &module = RedisProtobuf::instance();

    auto key = api::open_key(ctx, key_name, api::KeyMode::READWRITE);
    if (!api::key_exists(key.get(), module.type())) {
        if (RedisModule_ModuleTypeSetValue(key.get(), module.type(), msg.get()) != REDISMODULE_OK) {
            throw Error("failed to create message");
        }

        msg.release();
    } else {
        auto cur_msg = api::get_msg_by_key(key.get());
        assert(cur_msg != nullptr);

        cur_msg->MergeFrom(*msg);
    }
}

void SetCommand::_set_msg(gp::Message *msg,
        const std::string &type,
        const StringView &val) const {
    assert(msg != nullptr);

    auto &module = RedisProtobuf::instance();
    auto new_msg = module.proto_factory()->create(type, val);

    msg->GetReflection()->Swap(msg, new_msg.get());
}

void SetCommand::_set_field(gp::Message *msg,
        const Path &path,
        const StringView &val) const {
    assert(msg != nullptr);

    FieldRef field(msg, path);

    _set_field(field, val);
}

void SetCommand::_set_field(FieldRef &field, const StringView &val) const {
    if (field.field_desc->is_repeated()) {
        throw Error("cannot set repeated field");
    }

    if (field.field_desc->is_map()) {
        throw Error("cannot set map field");
    }

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

    field.msg->GetReflection()->SetString(field.msg, field.field_desc, std::string(sv.data(), sv.size()));
}

void SetCommand::_set_msg(FieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_MESSAGE);

    auto &module = RedisProtobuf::instance();

    auto new_msg = module.proto_factory()->create(field.msg->GetTypeName(), sv);

    field.msg->GetReflection()->Swap(field.msg, new_msg.get());
}


}

}

}
