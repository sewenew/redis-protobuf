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

#ifndef SEWENEW_REDISPROTOBUF_REDIS_PROTOBUF_H
#define SEWENEW_REDISPROTOBUF_REDIS_PROTOBUF_H

#include "module_api.h"
#include "proto_factory.h"

namespace sw {

namespace redis {

namespace pb {

constexpr int MODULE_VERSION = 0;

constexpr int ENCODING_VERSION = 0;

constexpr const char *MODULE_NAME = "PB";

constexpr const char *TYPE_NAME = "PROTOC-SW";

extern RedisModuleType *redis_proto;

extern std::unique_ptr<ProtoFactory> proto_factory;

void* rdb_load(RedisModuleIO *rdb, int encver);

void rdb_save(RedisModuleIO *rdb, void *value);

void aof_rewrite(RedisModuleIO *aof, RedisModuleString *key, void *value);

void free_msg(void *value);

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_REDIS_PROTOBUF_H
