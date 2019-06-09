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

#include "schema_command.h"
#include "errors.h"
#include "redis_protobuf.h"

namespace sw {

namespace redis {

namespace pb {

int SchemaCommand::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        assert(ctx != nullptr);

        auto args = _parse_args(argv, argc);
        const auto &type = args.type;

        auto *desc = RedisProtobuf::instance().proto_factory()->descriptor(type);
        if (desc == nullptr) {
            RedisModule_ReplyWithNull(ctx);
        } else {
            auto schema = desc->DebugString();
            RedisModule_ReplyWithStringBuffer(ctx, schema.data(), schema.size());
        }

        return REDISMODULE_OK;
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

SchemaCommand::Args SchemaCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc != 2) {
        throw WrongArityError();
    }

    auto type = StringView(argv[1]);

    return {std::string(type.data(), type.size())};
}

}

}

}
