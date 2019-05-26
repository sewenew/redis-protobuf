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
        const auto &paths = args.paths;

        auto key = api::open_key(ctx, args.key_name, api::KeyMode::READONLY);
        if (!api::key_exists(key.get(), RedisProtobuf::instance().type())) {
            _reply_with_nil(ctx, paths);
        } else {
            auto *msg = api::get_msg_by_key(key.get());
            assert(msg != nullptr);

            _reply_with_msg(ctx, *msg, paths);
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

    if (argc < 2) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];
    args.paths.reserve(argc - 2);

    std::string type;
    for (auto idx = 2; idx != argc; ++idx) {
        Path path(argv[idx]);
        if (type.empty()) {
            type = path.type();
        } else {
            if (type != path.type()) {
                throw Error("fields have different types");
            }
        }

        args.paths.push_back(std::move(path));
    }

    return args;
}

void GetCommand::_get_msg(RedisModuleCtx *ctx, const gp::Message &msg) const {
    auto json = util::msg_to_json(msg);
    RedisModule_ReplyWithStringBuffer(ctx, json.data(), json.size());
}

void GetCommand::_get_field(RedisModuleCtx *ctx, const FieldRef &field) const {
    gp::Message *sub_msg = field.msg;
    const gp::FieldDescriptor *field_desc = field.field_desc;

    assert(sub_msg != nullptr && field_desc != nullptr);

    const auto *reflection = sub_msg->GetReflection();
    switch (field_desc->cpp_type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32: {
        auto val = reflection->GetInt32(*sub_msg, field_desc);
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_INT64: {
        auto val = reflection->GetInt64(*sub_msg, field_desc);
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT32: {
        auto val = reflection->GetUInt32(*sub_msg, field_desc);
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT64: {
        auto val = reflection->GetUInt64(*sub_msg, field_desc);
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_DOUBLE: {
        auto val = reflection->GetDouble(*sub_msg, field_desc);
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_FLOAT: {
        auto val = reflection->GetFloat(*sub_msg, field_desc);
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_BOOL: {
        auto val = reflection->GetBool(*sub_msg, field_desc);
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_STRING: {
        auto val = reflection->GetString(*sub_msg, field_desc);
        RedisModule_ReplyWithStringBuffer(ctx, val.data(), val.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_MESSAGE: {
        auto json = util::msg_to_json(*sub_msg);
        RedisModule_ReplyWithStringBuffer(ctx, json.data(), json.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_ENUM: {
        // TODO: add enum support
        assert(false);
        break;
    }
    default:
        assert(false);
    }
}

void GetCommand::_reply_with_nil(RedisModuleCtx *ctx, const std::vector<Path> &paths) const {
    std::size_t result_size = paths.empty() ? 1 : paths.size();

    if (result_size == 1) {
        RedisModule_ReplyWithNull(ctx);
    } else {
        RedisModule_ReplyWithArray(ctx, result_size);
        for (std::size_t idx = 0; idx != result_size; ++idx) {
            RedisModule_ReplyWithNull(ctx);
        }
    }
}

void GetCommand::_reply_with_msg(RedisModuleCtx *ctx,
        gp::Message &msg,
        const std::vector<Path> &paths) const {
    if (paths.empty()) {
        auto json = util::msg_to_json(msg);
        RedisModule_ReplyWithStringBuffer(ctx, json.data(), json.size());

        return;
    }

    if (msg.GetTypeName() != paths[0].type()) {
        throw Error("type missmatch");
    }

    std::vector<FieldRef> fields;
    fields.reserve(paths.size());
    for (const auto &path : paths) {
        fields.emplace_back(&msg, path);
    }

    if (paths.size() > 1) {
        RedisModule_ReplyWithArray(ctx, paths.size());
    }

    for (const auto &field : fields) {
        if (!field) {
            _get_msg(ctx, msg);
        } else {
            _get_field(ctx, field);
        }
    }
}

}

}

}
