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

#ifndef SEWENEW_REDISPROTOBUF_SCHEMA_COMMANDS_H
#define SEWENEW_REDISPROTOBUF_SCHEMA_COMMANDS_H

#include <google/protobuf/descriptor.h>
#include "module_api.h"
#include "utils.h"

namespace sw {

namespace redis {

namespace pb {

// command: PB.SCHEMA type
// return:  Bulk string reply: return the schema of the specified type. If
//          the type doesn't exist, return a nil reply.
class SchemaCommand {
public:
    int run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;

private:
    struct Args {
        std::string type;
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_SCHEMA_COMMANDS_H
