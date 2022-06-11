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

#include "append_command.h"
#include "errors.h"
#include "redis_protobuf.h"

namespace sw {

namespace redis {

namespace pb {

int AppendCommand::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        assert(ctx != nullptr);

        auto args = _parse_args(argv, argc);
        const auto &path = args.path;
        if (path.empty()) {
            throw Error("can only call append on array");
        }

        auto key = api::open_key(ctx, args.key_name, api::KeyMode::WRITEONLY);
        assert(key);

        auto &m = RedisProtobuf::instance();

        long long len = 0;
        if (!api::key_exists(key.get(), m.type())) {
            auto msg = m.proto_factory()->create(path.type());
            MutableFieldRef field(msg.get(), path);
            len = _append(field, args.elements);

            if (RedisModule_ModuleTypeSetValue(key.get(),
                        m.type(),
                        msg.get()) != REDISMODULE_OK) {
                throw Error("failed to set message");
            }

            msg.release();
        } else {
            auto *msg = api::get_msg_by_key(key.get());
            assert(msg != nullptr);

            MutableFieldRef field(msg, path);
            // TODO: create a new message, and append to that message, then swap to this message.
            len = _append(field, args.elements);
        }

        RedisModule_ReplyWithLongLong(ctx, len);

        RedisModule_ReplicateVerbatim(ctx);

        return REDISMODULE_OK;
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

AppendCommand::Args AppendCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 5) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];
    args.path = Path(argv[2], argv[3]);
    args.elements.reserve(argc - 4);

    for (auto idx = 4; idx != argc; ++idx) {
        args.elements.emplace_back(argv[idx]);
    }

    return args;
}

long long AppendCommand::_append(MutableFieldRef &field,
        const std::vector<StringView> &elements) const {
    if (field.is_array() && !field.is_array_element()) {
        for (const auto &ele : elements) {
            _append_arr(field, ele);
        }

        return field.size();
    } else if (field.type() == gp::FieldDescriptor::CPPTYPE_STRING) {
        return _append_str(field, elements);
    } else {
        throw Error("not an array or string");
    }
}

void AppendCommand::_append_arr(MutableFieldRef &field, const StringView &val) const {
    assert(field.is_array() && !field.is_array_element());

    switch (field.type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32:
        field.add_int32(util::sv_to_int32(val));
        break;

    case gp::FieldDescriptor::CPPTYPE_INT64:
        field.add_int64(util::sv_to_int64(val));
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT32:
        field.add_uint32(util::sv_to_uint32(val));
        break;

    case gp::FieldDescriptor::CPPTYPE_UINT64:
        field.add_uint64(util::sv_to_uint64(val));
        break;

    case gp::FieldDescriptor::CPPTYPE_DOUBLE:
        field.add_double(util::sv_to_double(val));
        break;

    case gp::FieldDescriptor::CPPTYPE_FLOAT:
        field.add_float(util::sv_to_float(val));
        break;

    case gp::FieldDescriptor::CPPTYPE_BOOL:
        field.add_bool(util::sv_to_bool(val));
        break;

    case gp::FieldDescriptor::CPPTYPE_ENUM:
        field.add_enum(util::sv_to_int32(val));
        break;

    case gp::FieldDescriptor::CPPTYPE_STRING:
        field.add_string(util::sv_to_string(val));
        break;

    case gp::FieldDescriptor::CPPTYPE_MESSAGE:
        _add_msg(field, val);
        break;

    default:
        throw Error("unknown type");
    }
}

long long AppendCommand::_append_str(MutableFieldRef &field,
        const std::vector<StringView> &elements) const {
    std::string str;
    for (const auto &ele : elements) {
        str += std::string(ele.data(), ele.size());
    }

    if (field.is_array_element()) {
        str = field.get_repeated_string() + str;
        field.set_repeated_string(str);
    } else {
        // TODO: map element
        str = field.get_string() + str;
        field.set_string(str);
    }

    return str.size();
}

void AppendCommand::_add_msg(MutableFieldRef &field, const StringView &val) const {
    auto msg = RedisProtobuf::instance().proto_factory()->create(field.msg_type(), val);
    assert(msg);

    field.add_msg(*msg);
}

}

}

}
