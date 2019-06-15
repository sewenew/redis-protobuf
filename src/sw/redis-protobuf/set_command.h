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

#ifndef SEWENEW_REDISPROTOBUF_SET_COMMANDS_H
#define SEWENEW_REDISPROTOBUF_SET_COMMANDS_H

#include "module_api.h"
#include <string>
#include <vector>
#include <chrono>
#include "utils.h"
#include "field_ref.h"

namespace sw {

namespace redis {

namespace pb {

// command: PB.SET key [--NX|--XX] [--EX seconds | --PX milliseconds] type|path value
// return:  Integer reply: 1 if set successfully. 0, otherwise, e.g. option --NX has
//          been set, while key already exists.
// error:   If the type doesn't match the protobuf message type of the
//          key, or the path doesn't exist, return an error reply.
class SetCommand {
public:
    int run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;

private:
    struct Args {
        RedisModuleString *key_name;

        enum class Opt {
            NX = 0,
            XX,
            NONE
        };

        Opt opt = Opt::NONE;

        std::chrono::milliseconds expire{0};

        Path path;
        StringView val;
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;

    // Return the position of the first non-option argument.
    int _parse_opts(RedisModuleString **argv, int argc, Args &args) const;

    int64_t _parse_expire(const StringView &sv) const;

    void _create_msg(RedisModuleKey &key,
            const Path &path,
            const StringView &val) const;

    void _set_msg(RedisModuleKey &key,
            const Path &path,
            const StringView &val) const;

    void _set_scalar_field(FieldRef &field, const StringView &val) const;

    void _set_array_element(FieldRef &field, const StringView &val) const;

    void _set_field(FieldRef &field, const StringView &sv) const;

    void _set_int32(FieldRef &field, const StringView &sv) const;

    void _set_int64(FieldRef &field, const StringView &sv) const;

    void _set_uint32(FieldRef &field, const StringView &sv) const;

    void _set_uint64(FieldRef &field, const StringView &sv) const;

    void _set_double(FieldRef &field, const StringView &sv) const;

    void _set_float(FieldRef &field, const StringView &sv) const;

    void _set_bool(FieldRef &field, const StringView &sv) const;

    void _set_enum(FieldRef &field, const StringView &sv) const;

    void _set_string(FieldRef &field, const StringView &sv) const;

    void _set_msg(FieldRef &field, const StringView &sv) const;

    void _set_repeated_int32(FieldRef &field, const StringView &sv) const;

    void _set_repeated_int64(FieldRef &field, const StringView &sv) const;

    void _set_repeated_uint32(FieldRef &field, const StringView &sv) const;

    void _set_repeated_uint64(FieldRef &field, const StringView &sv) const;

    void _set_repeated_double(FieldRef &field, const StringView &sv) const;

    void _set_repeated_float(FieldRef &field, const StringView &sv) const;

    void _set_repeated_bool(FieldRef &field, const StringView &sv) const;

    void _set_repeated_enum(FieldRef &field, const StringView &sv) const;

    void _set_repeated_string(FieldRef &field, const StringView &sv) const;

    void _set_repeated_msg(FieldRef &field, const StringView &sv) const;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_SET_COMMANDS_H
