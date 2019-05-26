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
#include "utils.h"
#include "field_ref.h"

namespace sw {

namespace redis {

namespace pb {

// command: PB.SET key path value [path value ...]
// return:  Integer reply: 1 if set successfully. 0, otherwise.
class SetCommand {
public:
    int run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const;

private:
    struct Args {
        RedisModuleString *key_name;
        std::vector<std::pair<Path, StringView>> paths;
    };

    Args _parse_args(RedisModuleString **argv, int argc) const;

    MsgUPtr _build_msg(const std::vector<std::pair<Path, StringView>> &paths) const;

    void _set_msg(RedisModuleCtx *ctx, RedisModuleString *key_name, MsgUPtr msg) const;

    void _set_msg(gp::Message *msg, const std::string &type, const StringView &val) const;

    void _set_field(gp::Message *msg, const Path &path, const StringView &val) const;

    void _set_field(FieldRef &field, const StringView &sv) const;

    void _set_int32(FieldRef &field, const StringView &sv) const;

    void _set_int64(FieldRef &field, const StringView &sv) const;

    void _set_uint32(FieldRef &field, const StringView &sv) const;

    void _set_uint64(FieldRef &field, const StringView &sv) const;

    void _set_double(FieldRef &field, const StringView &sv) const;

    void _set_float(FieldRef &field, const StringView &sv) const;

    void _set_bool(FieldRef &field, const StringView &sv) const;

    void _set_string(FieldRef &field, const StringView &sv) const;

    void _set_msg(FieldRef &field, const StringView &sv) const;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_SET_COMMANDS_H
