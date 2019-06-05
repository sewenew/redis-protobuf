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

#ifndef SEWENEW_REDISPROTOBUF_LEN_COMMANDS_H
#define SEWENEW_REDISPROTOBUF_LEN_COMMANDS_H

#include "module_api.h"
#include "utils.h"
#include "field_ref.h"

namespace sw {

namespace redis {

namespace pb {

// command: PB.LEN key [path]
// return:  Integer reply: If no path is specified, return the length of the
//          protobuf message in bytes, i.e. Message::ByteSizeLong. If path is
//          specified, and the corresponding field is a string, return the 
//          length of the string in bytes; if the field is an array or a map,
//          return the size of the array or map. If the field is a message,
//          return the size of the message in bytes. If the key doesn't exist,
//          return 0.
// error:   If the path doesn't exist, or the corresponding field is not a
//          string or an array or a map, return an error reply.
class LenCommand {
public:
    int run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;

private:
    struct Args {
        RedisModuleString *key_name;
        // TODO: We might support multiple paths in the future.
        std::vector<Path> paths;
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;

    long long _len(gp::Message &msg, const std::vector<Path> &paths) const;

    long long _len(const FieldRef &field) const;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_LEN_COMMANDS_H
