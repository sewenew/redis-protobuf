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

#include "get_command.h"
#include "errors.h"
#include "redis_protobuf.h"
#include "utils.h"
#include "field_ref.h"

namespace sw {

namespace redis {

namespace pb {

int GetCommand::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        assert(ctx != nullptr);

        auto args = _parse_args(argv, argc);

        auto key = api::open_key(ctx, args.key_name, api::KeyMode::READONLY);
        if (!api::key_exists(key.get(), RedisProtobuf::instance().type())) {
            _reply_with_nil(ctx);
        } else {
            auto *msg = api::get_msg_by_key(key.get());
            assert(msg != nullptr);

            _reply_with_msg(ctx, *msg, args.paths);
        }

        return REDISMODULE_OK;
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }
}

GetCommand::Args GetCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc != 2 && argc != 3) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    // Get field.
    if (argc == 3) {
        args.paths.emplace_back(argv[2]);
    }

    return args;
}

void GetCommand::_get_msg(RedisModuleCtx *ctx, const gp::Message &msg) const {
    auto json = util::msg_to_json(msg);
    RedisModule_ReplyWithStringBuffer(ctx, json.data(), json.size());
}

void GetCommand::_get_field(RedisModuleCtx *ctx, const FieldRef &field) const {
    if (field.is_array_element()) {
        return _get_array_element(ctx, field);
    } else if (field.is_array()) {
        // TODO: 
        throw Error("cannot get array field");
    } else if (field.is_map()) {
        // TODO: add map support.
        throw Error("cannot get map field");
    } // else non-aggregate type.

    _get_scalar_field(ctx, field);
}

void GetCommand::_get_scalar_field(RedisModuleCtx *ctx, const FieldRef &field) const {
    switch (field.type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32: {
        auto val = field.get_int32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_INT64: {
        auto val = field.get_int64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT32: {
        auto val = field.get_uint32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT64: {
        auto val = field.get_uint64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_DOUBLE: {
        auto val = field.get_double();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_FLOAT: {
        auto val = field.get_float();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_BOOL: {
        auto val = field.get_bool();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_STRING: {
        auto val = field.get_string();
        RedisModule_ReplyWithStringBuffer(ctx, val.data(), val.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_MESSAGE: {
        auto json = util::msg_to_json(*field.msg());
        RedisModule_ReplyWithStringBuffer(ctx, json.data(), json.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_ENUM: {
        // TODO: add enum support
        throw Error("cannot get enum field");
        break;
    }
    default:
        assert(false);
    }
}

void GetCommand::_get_array_element(RedisModuleCtx *ctx, const FieldRef &field) const {
    switch (field.type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32: {
        auto val = field.get_repeated_int32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_INT64: {
        auto val = field.get_repeated_int64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT32: {
        auto val = field.get_repeated_uint32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT64: {
        auto val = field.get_repeated_uint64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_DOUBLE: {
        auto val = field.get_repeated_double();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_FLOAT: {
        auto val = field.get_repeated_float();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_BOOL: {
        auto val = field.get_repeated_bool();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_STRING: {
        const auto &val = field.get_repeated_string();
        RedisModule_ReplyWithStringBuffer(ctx, val.data(), val.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_MESSAGE: {
        const auto &msg = field.get_repeated_msg();
        auto json = util::msg_to_json(msg);
        RedisModule_ReplyWithStringBuffer(ctx, json.data(), json.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_ENUM: {
        // TODO: add enum support
        throw Error("cannot get enum field");
        break;
    }
    default:
        assert(false);
    }
}

void GetCommand::_reply_with_nil(RedisModuleCtx *ctx) const {
    RedisModule_ReplyWithNull(ctx);
}

void GetCommand::_reply_with_msg(RedisModuleCtx *ctx,
        gp::Message &msg,
        const std::vector<Path> &paths) const {
    if (paths.empty()) {
        // Get the whole message.
        return _get_msg(ctx, msg);
    }

    // Get field.
    const auto &path = paths.front();
    if (msg.GetTypeName() != path.type()) {
        throw Error("type missmatch");
    }

    if (path.empty()) {
        return _get_msg(ctx, msg);
    }

    _get_field(ctx, FieldRef(&msg, path));
}

}

}

}
