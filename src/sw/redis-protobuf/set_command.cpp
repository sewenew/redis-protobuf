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
        auto res = _run(ctx, argv, argc);

        RedisModule_ReplyWithLongLong(ctx, res);

        RedisModule_ReplicateVerbatim(ctx);

        return REDISMODULE_OK;
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

int SetCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    assert(ctx != nullptr);

    // TODO: if the ByteSize is too large, serialization might fail.

    auto args = _parse_args(argv, argc);
    const auto &path = args.path;

    auto key = api::open_key(ctx, args.key_name, api::KeyMode::WRITEONLY);
    assert(key);

    if (!api::key_exists(key.get(), RedisProtobuf::instance().type())) {
        if (args.opt == Args::Opt::XX) {
            return 0;
        }

        _create_msg(*key, path, args.val);
    } else {
        if (args.opt == Args::Opt::NX) {
            return 0;
        }

        _set_msg(*key, path, args.val);
    }

    auto expire = args.expire.count();
    if (expire > 0) {
        RedisModule_SetExpire(key.get(), expire);
    }

    return 1;
}

SetCommand::Args SetCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 4) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto pos = _parse_opts(argv, argc, args);
    if (pos + 2 != argc && pos + 3 != argc) {
        throw WrongArityError();
    }

    Path path;
    StringView val;
    if (pos + 2 == argc) {
        path = Path(argv[pos]);
        val = argv[pos + 1];
    } else {
        path = Path(argv[pos], argv[pos + 1]);
        val = argv[pos + 2];
    }

    args.path = std::move(path);
    args.val = std::move(val);

    return args;
}

int SetCommand::_parse_opts(RedisModuleString **argv, int argc, Args &args) const {
    auto idx = 2;
    while (idx < argc) {
        auto opt = StringView(argv[idx]);

        if (util::str_case_equal(opt, "--NX")) {
            if (args.opt != Args::Opt::NONE) {
                throw Error("syntax error");
            }

            args.opt = Args::Opt::NX;
        } else if (util::str_case_equal(opt, "--XX")) {
            if (args.opt != Args::Opt::NONE) {
                throw Error("syntax error");
            }

            args.opt = Args::Opt::XX;
        } else if (util::str_case_equal(opt, "--EX")) {
            if (args.expire != std::chrono::milliseconds(0) || idx + 1 >= argc) {
                throw Error("syntax error");
            }

            // NOTE: this is tricky, that we modified idx in the loop.
            ++idx;

            auto expire = _parse_expire(argv[idx]);
            args.expire = std::chrono::seconds(expire);
        } else if (util::str_case_equal(opt, "--PX")) {
            if (args.expire != std::chrono::milliseconds(0) || idx + 1 >= argc) {
                throw Error("syntax error");
            }

            // NOTE: this is tricky, that we modified idx in the loop.
            ++idx;

            auto expire = _parse_expire(argv[idx]);
            args.expire = std::chrono::milliseconds(expire);
        } else {
            // Finish parsing options.
            break;
        }

        ++idx;
    }

    return idx;
}

int64_t SetCommand::_parse_expire(const StringView &sv) const {
    auto expire = util::sv_to_int64(sv);
    if (expire <= 0) {
        throw Error("expire must larger than 0");
    }

    return expire;
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
        MutableFieldRef field(msg.get(), path);
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
    auto *msg = api::get_msg_by_key(&key);
    assert(msg != nullptr);

    if (path.empty()) {
        // Set the whole message.
        if (msg->GetTypeName() != path.type()) {
            throw Error("type mismatch");
        }

        auto &module = RedisProtobuf::instance();
        auto msg = module.proto_factory()->create(path.type(), val);
        if (RedisModule_ModuleTypeSetValue(&key, module.type(), msg.get()) != REDISMODULE_OK) {
            throw Error("failed to set message");
        }

        msg.release();
    } else {
        // Set field.
        MutableFieldRef field(msg, path);
        _set_field(field, val);
    }
}

void SetCommand::_set_field(MutableFieldRef &field, const StringView &val) const {
    if (field.is_map_element()) {
        return _set_map_element(field, val);
    } else if (field.is_map()) {
        throw Error("cannot set the whole map field");
    } else if (field.is_array_element()) {
        return _set_array_element(field, val);
    } else if (field.is_array()) {
        throw Error("cannot set the whole array field");
    }

    _set_scalar_field(field, val);
}

void SetCommand::_set_scalar_field(MutableFieldRef &field, const StringView &val) const {
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

    case gp::FieldDescriptor::CPPTYPE_ENUM:
        _set_enum(field, val);
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

void SetCommand::_set_map_element(MutableFieldRef &field, const StringView &val) const {
    switch (field.map_value_type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32:
        _set_mapped_int32(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_INT64:
        _set_mapped_int64(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT32:
        _set_mapped_uint32(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT64:
        _set_mapped_uint64(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_DOUBLE:
        _set_mapped_double(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_FLOAT:
        _set_mapped_float(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_BOOL:
        _set_mapped_bool(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_ENUM:
        _set_mapped_enum(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_STRING:
        _set_mapped_string(field, val);
        break;

    case gp::FieldDescriptor::CPPTYPE_MESSAGE:
        _set_mapped_msg(field, val);
        break;

    default:
        throw Error("unknown type");
    }
}

void SetCommand::_set_array_element(MutableFieldRef &field, const StringView &val) const {
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

    case gp::FieldDescriptor::CPPTYPE_ENUM:
        _set_repeated_enum(field, val);
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

void SetCommand::_set_int32(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_INT32);

    auto val = util::sv_to_int32(sv);
    field.set_int32(val);
}

void SetCommand::_set_int64(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_INT64);

    auto val = util::sv_to_int64(sv);
    field.set_int64(val);
}

void SetCommand::_set_uint32(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_UINT32);

    auto val = util::sv_to_uint32(sv);
    field.set_uint32(val);
}

void SetCommand::_set_uint64(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_UINT64);

    auto val = util::sv_to_uint64(sv);
    field.set_uint64(val);
}

void SetCommand::_set_double(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_DOUBLE);

    auto val = util::sv_to_double(sv);
    field.set_double(val);
}

void SetCommand::_set_float(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_FLOAT);

    auto val = util::sv_to_float(sv);
    field.set_float(val);
}

void SetCommand::_set_bool(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_BOOL);

    auto val = util::sv_to_bool(sv);
    field.set_bool(val);
}

void SetCommand::_set_enum(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_ENUM);

    auto val = util::sv_to_int32(sv);
    field.set_enum(val);
}

void SetCommand::_set_string(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_STRING);

    field.set_string(util::sv_to_string(sv));
}

void SetCommand::_set_msg(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_MESSAGE);

    auto new_msg = RedisProtobuf::instance().proto_factory()->create(field.msg_type(), sv);
    assert(new_msg);

    field.set_msg(*new_msg);
}

void SetCommand::_set_repeated_int32(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_INT32);

    auto val = util::sv_to_int32(sv);
    field.set_repeated_int32(val);
}

void SetCommand::_set_repeated_int64(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_INT64);

    auto val = util::sv_to_int64(sv);
    field.set_repeated_int64(val);
}

void SetCommand::_set_repeated_uint32(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_UINT32);

    auto val = util::sv_to_uint32(sv);
    field.set_repeated_uint32(val);
}

void SetCommand::_set_repeated_uint64(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_UINT64);

    auto val = util::sv_to_uint64(sv);
    field.set_repeated_uint64(val);
}

void SetCommand::_set_repeated_double(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_DOUBLE);

    auto val = util::sv_to_double(sv);
    field.set_repeated_double(val);
}

void SetCommand::_set_repeated_float(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_FLOAT);

    auto val = util::sv_to_float(sv);
    field.set_repeated_float(val);
}

void SetCommand::_set_repeated_bool(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_BOOL);

    auto val = util::sv_to_bool(sv);
    field.set_repeated_bool(val);
}

void SetCommand::_set_repeated_enum(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_ENUM);

    auto val = util::sv_to_int32(sv);
    field.set_repeated_enum(val);
}

void SetCommand::_set_repeated_string(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_STRING);

    field.set_repeated_string(util::sv_to_string(sv));
}

void SetCommand::_set_repeated_msg(MutableFieldRef &field, const StringView &sv) const {
    assert(field.type() == gp::FieldDescriptor::CPPTYPE_MESSAGE);

    auto new_msg = RedisProtobuf::instance().proto_factory()->create(field.msg_type(), sv);
    assert(new_msg);

    field.set_repeated_msg(*new_msg);
}

void SetCommand::_set_mapped_int32(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_INT32);

    auto val = util::sv_to_int32(sv);
    field.set_mapped_int32(val);
}

void SetCommand::_set_mapped_int64(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_INT64);

    auto val = util::sv_to_int64(sv);
    field.set_mapped_int64(val);
}

void SetCommand::_set_mapped_uint32(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_UINT32);

    auto val = util::sv_to_uint32(sv);
    field.set_mapped_uint32(val);
}

void SetCommand::_set_mapped_uint64(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_UINT64);

    auto val = util::sv_to_uint64(sv);
    field.set_mapped_uint64(val);
}

void SetCommand::_set_mapped_double(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_DOUBLE);

    auto val = util::sv_to_double(sv);
    field.set_mapped_double(val);
}

void SetCommand::_set_mapped_float(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_FLOAT);

    auto val = util::sv_to_float(sv);
    field.set_mapped_float(val);
}

void SetCommand::_set_mapped_bool(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_BOOL);

    auto val = util::sv_to_bool(sv);
    field.set_mapped_bool(val);
}

void SetCommand::_set_mapped_enum(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_ENUM);

    auto val = util::sv_to_int32(sv);
    field.set_mapped_enum(val);
}

void SetCommand::_set_mapped_string(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_STRING);

    field.set_mapped_string(util::sv_to_string(sv));
}

void SetCommand::_set_mapped_msg(MutableFieldRef &field, const StringView &sv) const {
    assert(field.map_value_type() == gp::FieldDescriptor::CPPTYPE_MESSAGE);

    auto new_msg = RedisProtobuf::instance().proto_factory()->create(field.mapped_msg_type(), sv);
    assert(new_msg);

    field.set_mapped_msg(*new_msg);
}

}

}

}
