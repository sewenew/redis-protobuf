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

#ifndef SEWENEW_REDISPROTOBUF_LAST_IMPORT_COMMAND_H
#define SEWENEW_REDISPROTOBUF_LAST_IMPORT_COMMAND_H

#include "module_api.h"
#include <string>
#include <unordered_map>
#include <vector>
#include "utils.h"
#include "field_ref.h"

namespace sw {

namespace redis {

namespace pb {

// command: PB.LASTIMPORT
// return:  OK status reply.
// error:   If failing to import, return an error reply.
class LastImportCommand {
public:
    int run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_LAST_IMPORT_COMMAND_H
