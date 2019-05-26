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

#ifndef SEWENEW_REDISPROTOBUF_MODULE_API_H
#define SEWENEW_REDISPROTOBUF_MODULE_API_H

#ifdef __cplusplus

extern "C" {

#endif

#include "redismodule.h"

#ifdef __cplusplus

}

#endif

#include <memory>
#include <google/protobuf/message.h>
#include "errors.h"

namespace sw {

namespace redis {

namespace pb {

namespace api {

template <typename ...Args>
void warning(RedisModuleCtx *ctx, Args &&...args) {
    RedisModule_Log(ctx, "warning", std::forward<Args>(args)...);
}

template <typename ...Args>
void notice(RedisModuleCtx *ctx, Args &&...args) {
    RedisModule_Log(ctx, "notice", std::forward<Args>(args)...);
}

template <typename ...Args>
void debug(RedisModuleCtx *ctx, Args &&...args) {
    RedisModule_Log(ctx, "debug", std::forward<Args>(args)...);
}

template <typename ...Args>
void verbose(RedisModuleCtx *ctx, Args &&...args) {
    RedisModule_Log(ctx, "verbose", std::forward<Args>(args)...);
}

struct RedisKeyCloser {
    void operator()(RedisModuleKey *key) const {
        RedisModule_CloseKey(key);
    }
};

using RedisKey = std::unique_ptr<RedisModuleKey, RedisKeyCloser>;

enum class KeyMode {
    READONLY,
    WRITEONLY,
    READWRITE
};

RedisKey open_key(RedisModuleCtx *ctx, RedisModuleString *name, KeyMode mode);

// If key doesn't exist return false.
// If key type is NOT *key_type*, throw WrongTypeError.
// Otherwise, return true.
bool key_exists(RedisModuleKey *key, RedisModuleType *key_type);

int reply_with_error(RedisModuleCtx *ctx, const Error &err);

google::protobuf::Message* get_msg_by_key(RedisModuleKey *key);

}

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_MODULE_API_H
