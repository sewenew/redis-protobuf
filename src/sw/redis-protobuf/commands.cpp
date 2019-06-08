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
#include "set_command.h"
#include "get_command.h"
#include "type_command.h"
#include "clear_command.h"
#include "len_command.h"
#include "append_command.h"

namespace sw {

namespace redis {

namespace pb {

namespace cmd {

void create_commands(RedisModuleCtx *ctx) {
    if (RedisModule_CreateCommand(ctx,
                "PB.TYPE",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    TypeCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("failed to create PB.TYPE command");
    }

    if (RedisModule_CreateCommand(ctx,
                "PB.SET",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    SetCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "write deny-oom",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("fail to create PB.SET command");
    }

    if (RedisModule_CreateCommand(ctx,
                "PB.GET",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    GetCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("failed to create PB.GET command");
    }

    if (RedisModule_CreateCommand(ctx,
                "PB.CLEAR",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    ClearCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "write deny-oom",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("fail to create PB.CLEAR command");
    }

    if (RedisModule_CreateCommand(ctx,
                "PB.LEN",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    LenCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("failed to create PB.LEN command");
    }

    if (RedisModule_CreateCommand(ctx,
                "PB.APPEND",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    AppendCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "write deny-oom",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("fail to create PB.APPEND command");
    }
}

}

}

}

}
