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

#include "merge_command.h"
#include "errors.h"
#include "redis_protobuf.h"
#include "utils.h"
#include "field_ref.h"
#include "set_command.h"

namespace sw {

namespace redis {

namespace pb {

int MergeCommand::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        assert(ctx != nullptr);

        auto args = _parse_args(argv, argc);

        auto key = api::open_key(ctx, args.key_name, api::KeyMode::WRITEONLY);
        if (!api::key_exists(key.get(), RedisProtobuf::instance().type())) {
            SetCommand set_cmd;
            return set_cmd.run(ctx, argv, argc);
        }

        auto *msg = api::get_msg_by_key(key.get());
        assert(msg != nullptr);

        _merge(args, *msg);

        return RedisModule_ReplyWithLongLong(ctx, 0);
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

MergeCommand::Args MergeCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc != 4) {
        throw WrongArityError();
    }

    return {argv[1], Path(argv[2]), StringView(argv[3])};
}

void MergeCommand::_merge(const Args &args, gp::Message &msg) const {
    const auto &path = args.path;
    if (path.empty()) {
        _merge_msg(path.type(), args.val, msg);
    } else {
        _merge_sub_msg(path, args.val, msg);
    }
}

void MergeCommand::_merge_msg(const std::string &type,
        const StringView &val,
        gp::Message &msg) const {
    if (type != msg.GetTypeName()) {
        throw Error("type mismatch");
    }

    auto other = RedisProtobuf::instance().proto_factory()->create(type, val);
    assert(other);

    msg.MergeFrom(*other);
}

void MergeCommand::_merge_sub_msg(const Path &path,
        const StringView &val,
        gp::Message &msg) const {
    FieldRef field(&msg, path);
    auto sub_msg = RedisProtobuf::instance().proto_factory()->create(field.msg_type(), val);
    assert(sub_msg);

    field.merge(*sub_msg);
}

}

}

}
