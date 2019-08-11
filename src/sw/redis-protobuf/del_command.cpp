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

#include "del_command.h"
#include "errors.h"
#include "redis_protobuf.h"

namespace sw {

namespace redis {

namespace pb {

int DelCommand::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        assert(ctx != nullptr);

        auto args = _parse_args(argv, argc);

        auto key = api::open_key(ctx, args.key_name, api::KeyMode::WRITEONLY);
        assert(key);

        if (!api::key_exists(key.get(), RedisProtobuf::instance().type())) {
            RedisModule_ReplyWithLongLong(ctx, 0);
        } else {
            auto *msg = api::get_msg_by_key(key.get());
            assert(msg != nullptr);

            const auto &path = args.path;
            if (msg->GetTypeName() != path.type()) {
                throw Error("type mismatch");
            }

            if (path.empty()) {
                // Delete key.
                RedisModule_DeleteKey(key.get());
            } else {
                // Delete an item from array or map.
                _del(*msg, path);
            }

            RedisModule_ReplyWithLongLong(ctx, 1);
        }

        RedisModule_ReplicateVerbatim(ctx);

        return REDISMODULE_OK;
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

DelCommand::Args DelCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc != 3) {
        throw WrongArityError();
    }

    return {argv[1], Path(argv[2])};
}

void DelCommand::_del(gp::Message &msg, const Path &path) const {
    MutableFieldRef field(&msg, path);

    if (!field.is_array_element()) {
        // TODO: support map element
        throw Error("not an array or map");
    }

    field.del();
}

}

}

}
