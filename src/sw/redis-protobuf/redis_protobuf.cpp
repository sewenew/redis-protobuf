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

#include "redis_protobuf.h"
#include <cassert>
#include <string>
#include <google/protobuf/message.h>
#include "proto_factory.h"
#include "errors.h"

namespace {

std::pair<std::string, std::string> serialize_message(void *value);

}

namespace sw {

namespace redis {

namespace pb {

RedisModuleType *redis_proto = nullptr;

std::unique_ptr<ProtoFactory> proto_factory;

void* rdb_load(RedisModuleIO *rdb, int encver) {
    assert(rdb != nullptr);

    if (encver != ENCODING_VERSION) {
        RedisModule_LogIOError(rdb, "warning", "cannot load protobuf of version: %d", encver);
        return nullptr;
    }

    std::size_t len = 0;
    auto *buf = RedisModule_LoadStringBuffer(rdb, &len);
    if (buf == nullptr) {
        RedisModule_LogIOError(rdb, "warning", "failed to load string buffer for type info");
        return nullptr;
    }

    auto type = std::string(buf, len);

    assert(proto_factory);

    auto msg = proto_factory->create(type);
    if (!msg) {
        RedisModule_LogIOError(rdb,
                                "warning",
                                "unknown protobuf type: %b",
                                type.data(),
                                type.size());
        return nullptr;
    }

    buf = RedisModule_LoadStringBuffer(rdb, &len);
    if (buf == nullptr) {
        RedisModule_LogIOError(rdb, "warning", "failed to load string buffer for protobuf data");
        return nullptr;
    }

    if (!msg->ParseFromArray(buf, len)) {
        RedisModule_LogIOError(rdb,
                                "warning",
                                "failed to parse protobuf of type: %b",
                                type.data(),
                                type.size());
        return nullptr;
    }

    return msg.release();
}

void rdb_save(RedisModuleIO *rdb, void *value) {
    try {
        assert(rdb != nullptr);

        std::string type;
        std::string buf;
        std::tie(type, buf) = serialize_message(value);

        RedisModule_SaveStringBuffer(rdb, type.data(), type.size());

        RedisModule_SaveStringBuffer(rdb, buf.data(), buf.size());
    } catch (const Error &e) {
        RedisModule_LogIOError(rdb, "warning", e.what());
    }
}

void aof_rewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    try {
        assert(aof != nullptr);

        if (key == nullptr) {
            throw Error("null key to rewrite aof");
        }

        std::string type;
        std::string buf;
        std::tie(type, buf) = serialize_message(value);

        RedisModule_EmitAOF(aof,
                            "PB.SET",
                            "sbb",
                            key,
                            type.data(),
                            type.size(),
                            buf.data(),
                            buf.size());
    } catch (const Error &e) {
        RedisModule_LogIOError(aof, "warning", e.what());
    }
}

void free_msg(void *value) {
    if (value != nullptr) {
        auto *msg = static_cast<gpb::Message*>(value);
        delete msg;
    }
}

}

}

}

namespace {

std::pair<std::string, std::string> serialize_message(void *value) {
    if (value == nullptr) {
        throw sw::redis::pb::Error("Null value to serialize");
    }

    auto *msg = static_cast<sw::redis::pb::gpb::Message*>(value);

    auto type = msg->GetTypeName();

    std::string buf;
    if (!msg->SerializeToString(&buf)) {
        throw sw::redis::pb::Error("failed to serialize protobuf message of type " + type);
    }

    return {std::move(type), std::move(buf)};
}

}
