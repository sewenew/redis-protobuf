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

#include "clear_command.h"
#include "errors.h"
#include "redis_protobuf.h"

namespace sw {

namespace redis {

namespace pb {

int ClearCommand::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        assert(ctx != nullptr);

        auto args = _parse_args(argv, argc);

        auto key = api::open_key(ctx, args.key_name, api::KeyMode::READONLY);
        if (!api::key_exists(key.get(), RedisProtobuf::instance().type())) {
            RedisModule_ReplyWithLongLong(ctx, 0);
        } else {
            auto *msg = api::get_msg_by_key(key.get());
            assert(msg != nullptr);

            _clear(*msg, args.paths);

            RedisModule_ReplyWithLongLong(ctx, 1);
        }

        return REDISMODULE_OK;
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

ClearCommand::Args ClearCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc != 2 && argc != 3) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    if (argc == 3) {
        args.paths.emplace_back(argv[2]);
    }

    return args;
}

void ClearCommand::_clear(gp::Message &msg, const std::vector<Path> &paths) const {
    if (paths.empty()) {
        // Clear the message.
        msg.Clear();
    } else {
        // Clear a field.
        FieldRef field(&msg, paths.front());
        field.clear();
    }
}

}

}

}
