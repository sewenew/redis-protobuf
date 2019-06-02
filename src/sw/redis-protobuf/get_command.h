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

#ifndef SEWENEW_REDISPROTOBUF_GET_COMMANDS_H
#define SEWENEW_REDISPROTOBUF_GET_COMMANDS_H

#include "module_api.h"
#include "utils.h"
#include "field_ref.h"

namespace sw {

namespace redis {

namespace pb {

// command: PB.GET key [path]
// return:  If no path is specified, return the protobuf message of the key
//          as a bulk string reply. If path is specified, return the value
//          of the field specified with the path, and the reply type depends
//          on the definition of the protobuf. If the key doesn't exist,
//          return a nil reply.
// error:   If the path doesn't exist, return an error reply.
class GetCommand {
public:
    int run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;

private:
    struct Args {
        RedisModuleString *key_name;
        // TODO: We might support multiple paths in the future.
        std::vector<Path> paths;
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;

    void _reply_with_nil(RedisModuleCtx *ctx) const;

    void _reply_with_msg(RedisModuleCtx *ctx,
            gp::Message &msg,
            const std::vector<Path> &paths) const;

    void _get_scalar_field(RedisModuleCtx *ctx, const FieldRef &field) const;

    void _get_array_element(RedisModuleCtx *ctx, const FieldRef &field) const;

    void _get_msg(RedisModuleCtx *ctx, const gp::Message &msg) const;

    void _get_field(RedisModuleCtx *ctx, const FieldRef &field) const;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_GET_COMMANDS_H
