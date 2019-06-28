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

#ifndef SEWENEW_REDISPROTOBUF_TYPE_COMMANDS_H
#define SEWENEW_REDISPROTOBUF_TYPE_COMMANDS_H

#include "module_api.h"
#include <string>
#include <vector>
#include "utils.h"

namespace sw {

namespace redis {

namespace pb {

// command: PB.TYPE key
// return:  Simple string reply: the protobuf type of the key.
//          If key doesn't exist, return a nil reply.
class TypeCommand {
public:
    int run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;

private:
    struct Args {
        RedisModuleString *key_name;
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;

    std::string _format_type(std::string type) const;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_TYPE_COMMANDS_H
