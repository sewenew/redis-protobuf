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

#include "commands.h"
#include <cassert>
#include "errors.h"
#include "redis_protobuf.h"

namespace {

int ReplyWithError(RedisModuleCtx *ctx, const std::string &err);

}

namespace sw {

namespace redis {

namespace pb {

namespace cmd {

// PB.TYPE key [field]
int type_command(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    try {
        assert(ctx != nullptr);

        auto &module = RedisProtobuf::instance();

        if (argc != 2 && argc != 3) {
            return RedisModule_WrongArity(ctx);
        }

        auto key = api::open_key(ctx, argv[1], api::KeyMode::READONLY);
        if (!api::key_exists(key.get(), module.type())) {
            return RedisModule_ReplyWithNull(ctx);
        }

        // get type info
        return RedisModule_ReplyWithSimpleString(ctx, "type");
    } catch (const Error &e) {
        return ReplyWithError(ctx, e.what());
    }
}

}

void create_commands(RedisModuleCtx *ctx) {
    if (RedisModule_CreateCommand(ctx,
                "PB.TYPE",
                cmd::type_command,
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("failed to create type command");
    }
    /*
    if (RedisModule_CreateCommand(ctx,
                                    "PB.SET",
                                    set_command,
                                    "write deny-oom",
                                    1,
                                    1,
                                    1) == REDISMODULE_ERR) {
        throw rpb::Error("fail to create set command");
    }
    */
}

}

}

}

namespace {

int ReplyWithError(RedisModuleCtx *ctx, const std::string &err) {
    assert(ctx != nullptr);

    auto msg = "ERR " + err;

    return RedisModule_ReplyWithError(ctx, msg.c_str());
}

}
