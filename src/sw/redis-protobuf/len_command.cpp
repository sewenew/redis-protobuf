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

#include "len_command.h"
#include "errors.h"
#include "redis_protobuf.h"
#include "utils.h"
#include "field_ref.h"

namespace sw {

namespace redis {

namespace pb {

int LenCommand::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        assert(ctx != nullptr);

        auto args = _parse_args(argv, argc);

        auto key = api::open_key(ctx, args.key_name, api::KeyMode::READONLY);
        if (!api::key_exists(key.get(), RedisProtobuf::instance().type())) {
            RedisModule_ReplyWithLongLong(ctx, 0);
        } else {
            auto *msg = api::get_msg_by_key(key.get());
            assert(msg != nullptr);

            auto len = _len(*msg, args.path);
            RedisModule_ReplyWithLongLong(ctx, len);
        }

        return REDISMODULE_OK;
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }
}

LenCommand::Args LenCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc != 3) {
        throw WrongArityError();
    }

    return {argv[1], Path(argv[2])};
}

long long LenCommand::_len(gp::Message &msg, const Path &path) const {
    if (msg.GetTypeName() != path.type()) {
        throw Error("type mismatch");
    }

    if (path.empty()) {
        // Return the length of the message.
        return msg.ByteSizeLong();
    }

    return _len(FieldRef(&msg, path));
}

long long LenCommand::_len(const FieldRef &field) const {
    if (field.is_array()) {
        return field.array_size();
    }

    if (field.is_map()) {
        throw Error("map not supported yet");
    }

    // Scalar type.
    switch (field.type()) {
    case gp::FieldDescriptor::CPPTYPE_MESSAGE:
        return field.get_msg().ByteSizeLong();

    case gp::FieldDescriptor::CPPTYPE_STRING:
        // TODO: use GetStringReference instead.
        return field.get_string().size();

    default:
        throw Error("cannot get length of this field");
    }
}

}

}

}
