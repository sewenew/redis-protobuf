/**************************************************************************
   Copyright (c) 2022 sewenew

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

#include "last_import_command.h"
#include <cassert>
#include "utils.h"
#include "errors.h"
#include "redis_protobuf.h"

namespace sw {

namespace redis {

namespace pb {

int LastImportCommand::run(RedisModuleCtx *ctx, RedisModuleString ** /*argv*/, int /*argc*/) const {
    try {
        assert(ctx != nullptr);

        auto &module = RedisProtobuf::instance();
        auto last_loaded_files = module.proto_factory()->last_loaded();

        RedisModule_ReplyWithArray(ctx, last_loaded_files.size() * 2);

        for (const auto &ele : last_loaded_files) {
            const auto &filename = ele.first;
            const auto &status = ele.second;
            RedisModule_ReplyWithStringBuffer(ctx, filename.data(), filename.size());
            RedisModule_ReplyWithStringBuffer(ctx, status.data(), status.size());
        }

        return REDISMODULE_OK;
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

}

}

}
