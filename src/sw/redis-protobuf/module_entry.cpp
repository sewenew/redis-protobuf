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

#include "module_entry.h"
#include <unistd.h>
#include <cassert>
#include <string>
#include "redis_protobuf.h"
#include "errors.h"

namespace {

struct Options {
    std::string proto_dir;
};

Options parse_options(RedisModuleString **argv, int argc);

void create_commands(RedisModuleCtx *ctx);

}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    namespace rpb = sw::redis::pb;

    assert(ctx != nullptr);

    try {
        if (RedisModule_Init(ctx,
                                rpb::MODULE_NAME,
                                rpb::MODULE_VERSION,
                                REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
            throw rpb::Error("fail to init redis module");
        }

        auto opts = parse_options(argv, argc);

        RedisModuleTypeMethods tm = {
            .version = REDISMODULE_TYPE_METHOD_VERSION,
            .rdb_load = rpb::rdb_load,
            .rdb_save = rpb::rdb_save,
            .aof_rewrite = rpb::aof_rewrite,
            .free = rpb::free_msg
        };

        rpb::redis_proto = RedisModule_CreateDataType(ctx,
                                                        rpb::TYPE_NAME,
                                                        rpb::ENCODING_VERSION,
                                                        &tm);
        if (rpb::redis_proto == nullptr) {
            throw rpb::Error(std::string("failed to create ") + rpb::TYPE_NAME + "type");
        }

        rpb::proto_factory = std::unique_ptr<rpb::ProtoFactory>(
                                new rpb::ProtoFactory(opts.proto_dir));

        create_commands(ctx);

    } catch (const rpb::Error &e) {
        RedisModule_Log(ctx, "warning", e.what());
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

namespace {

namespace rpb = sw::redis::pb;

Options parse_options(RedisModuleString **argv, int argc) {
    std::vector<char*> argvs;
    argvs.reserve(argc + 1);
    argvs.push_back(const_cast<char*>(rpb::MODULE_NAME));
    for (auto idx = 0; idx != argc; ++idx) {
        argvs.push_back(const_cast<char*>(RedisModule_StringPtrLen(argv[idx], nullptr)));
    }

    Options opts;
    int opt = 0;
    while ((opt = getopt(argc + 1, argvs.data(), "d:")) != -1) {
        try {
            switch (opt) {
            case 'd':
                opts.proto_dir = optarg;
                break;

            default:
                throw rpb::Error("Unknown command line option");
                break;
            }
        } catch (const rpb::Error &e) {
            throw;
        }
    }

    if (opts.proto_dir.empty()) {
        throw rpb::Error("proto directory is not specified with -p option");
    }

    return opts;
}

void create_commands(RedisModuleCtx * /*ctx*/) {
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

    if (RedisModule_CreateCommand(ctx,
                                    "PB.GET",
                                    get_command,
                                    "readonly",
                                    1,
                                    1,
                                    1) == REDISMODULE_ERR) {
        throw rpb::Error("fail to create get command");
    }
    */
}

}
