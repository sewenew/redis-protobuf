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

#include "get_command.h"
#include "errors.h"
#include "redis_protobuf.h"
#include "utils.h"
#include "field_ref.h"

namespace sw {

namespace redis {

namespace pb {

int GetCommand::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        assert(ctx != nullptr);

        auto args = _parse_args(argv, argc);

        auto key = api::open_key(ctx, args.key_name, api::KeyMode::READONLY);
        if (!api::key_exists(key.get(), RedisProtobuf::instance().type())) {
            _reply_with_nil(ctx);
        } else {
            auto *msg = api::get_msg_by_key(key.get());
            assert(msg != nullptr);

            _reply_with_msg(ctx, *msg, args);
        }

        return REDISMODULE_OK;
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }
}

GetCommand::Args GetCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 3) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto pos = _parse_opts(argv, argc, args);
    if (pos + 1 != argc && pos + 2 != argc) {
        throw WrongArityError();
    }

    if (pos + 2 == argc) {
        args.path = Path(argv[pos], argv[pos + 1]);
    } else {
        args.path = Path(argv[pos]);
    }

    return args;
}

int GetCommand::_parse_opts(RedisModuleString **argv, int argc, Args &args) const {
    auto idx = 2;
    while (idx < argc) {
        auto opt = StringView(argv[idx]);
        if (util::str_case_equal(opt, "--FORMAT")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }

            ++idx;

            args.format = _parse_format(argv[idx]);
        } else {
            // Finish parsing options.
            break;
        }

        ++idx;
    }

    return idx;
}

GetCommand::Args::Format GetCommand::_parse_format(const StringView &format) const {
    if (util::str_case_equal(format, "BINARY")) {
        return Args::Format::BINARY;
    } else if (util::str_case_equal(format, "JSON")) {
        return Args::Format::JSON;
    } else {
        throw Error("unknown format");
    }
}

void GetCommand::_get_msg(RedisModuleCtx *ctx,
        const gp::Message &msg,
        Args::Format format) const {
    std::string result;
    switch (format) {
    case Args::Format::BINARY:
        if (!msg.SerializeToString(&result)) {
            throw Error("failed to serialize message to binary string");
        }
        break;

    case Args::Format::JSON:
        result = util::msg_to_json(msg);
        break;

    case Args::Format::NONE:
        throw Error("option --FORMAT not specified");
        break;

    default:
        assert(false);
    }

    RedisModule_ReplyWithStringBuffer(ctx, result.data(), result.size());
}

void GetCommand::_get_field(RedisModuleCtx *ctx,
        const ConstFieldRef &field,
        Args::Format format) const {
    if (field.is_map_element()) {
        _get_map_element(ctx, field, format);
    } else if (field.is_map()) {
        _get_map(ctx, field, format);
    } else if (field.is_array_element()) {
        _get_array_element(ctx, field, format);
    } else if (field.is_array()) {
        _get_array(ctx, field, format);
    } else {
        // Non-aggregate type.
        _get_scalar_field(ctx, field, format);
    }
}

void GetCommand::_get_scalar_field(RedisModuleCtx *ctx,
        const ConstFieldRef &field,
        Args::Format format) const {
    switch (field.type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32: {
        auto val = field.get_int32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_INT64: {
        auto val = field.get_int64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT32: {
        auto val = field.get_uint32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT64: {
        auto val = field.get_uint64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_DOUBLE: {
        auto val = field.get_double();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_FLOAT: {
        auto val = field.get_float();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_BOOL: {
        auto val = field.get_bool();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_ENUM: {
        auto val = field.get_enum();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_STRING: {
        auto val = field.get_string();
        RedisModule_ReplyWithStringBuffer(ctx, val.data(), val.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_MESSAGE: {
        _get_msg(ctx, field.get_msg(), format);
        break;
    }
    default:
        assert(false);
    }
}

void GetCommand::_get_array_element(RedisModuleCtx *ctx,
        const ConstFieldRef &field,
        Args::Format format) const {
    switch (field.type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32: {
        auto val = field.get_repeated_int32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_INT64: {
        auto val = field.get_repeated_int64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT32: {
        auto val = field.get_repeated_uint32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT64: {
        auto val = field.get_repeated_uint64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_DOUBLE: {
        auto val = field.get_repeated_double();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_FLOAT: {
        auto val = field.get_repeated_float();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_BOOL: {
        auto val = field.get_repeated_bool();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_ENUM: {
        auto val = field.get_repeated_enum();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_STRING: {
        const auto &val = field.get_repeated_string();
        RedisModule_ReplyWithStringBuffer(ctx, val.data(), val.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_MESSAGE: {
        _get_msg(ctx, field.get_repeated_msg(), format);
        break;
    }
    default:
        assert(false);
    }
}

void GetCommand::_get_array(RedisModuleCtx *ctx,
        const ConstFieldRef &field,
        Args::Format format) const {
    auto arr_size = field.size();

    RedisModule_ReplyWithArray(ctx, arr_size);

    for (auto idx = 0; idx != arr_size; ++idx) {
        try {
            _get_field(ctx, field.get_array_element(idx), format);
        } catch (const Error &e) {
            api::reply_with_error(ctx, e);
        }
    }
}

void GetCommand::_get_map_element(RedisModuleCtx *ctx,
        const ConstFieldRef &field,
        Args::Format format) const {
    switch (field.map_value_type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32: {
        auto val = field.get_mapped_int32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_INT64: {
        auto val = field.get_mapped_int64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT32: {
        auto val = field.get_mapped_uint32();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT64: {
        auto val = field.get_mapped_uint64();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_DOUBLE: {
        auto val = field.get_mapped_double();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_FLOAT: {
        auto val = field.get_mapped_float();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_BOOL: {
        auto val = field.get_mapped_bool();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_ENUM: {
        auto val = field.get_mapped_enum();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_STRING: {
        const auto &val = field.get_mapped_string();
        RedisModule_ReplyWithStringBuffer(ctx, val.data(), val.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_MESSAGE: {
        _get_msg(ctx, field.get_mapped_msg(), format);
        break;
    }
    default:
        assert(false);
    }
}

void GetCommand::_get_map(RedisModuleCtx *ctx,
        const ConstFieldRef &field,
        Args::Format format) const {
    auto arr_size = field.size();

    RedisModule_ReplyWithArray(ctx, arr_size);

    auto range = field.get_map_range();
    for (auto iter = range.first; iter != range.second; ++iter) {
        const auto &key = iter->first;
        const auto &val = iter->second;

        try {
            _get_map_kv(ctx, field, format, key, val);
        } catch (const Error &e) {
            api::reply_with_error(ctx, e);
        }
    }
}

void GetCommand::_get_map_kv(RedisModuleCtx *ctx,
        const ConstFieldRef &field,
        Args::Format format,
        const gp::MapKey &key,
        const gp::MapValueRef &value) const {
    RedisModule_ReplyWithArray(ctx, 2);

    // Reply with key.
    switch (key.type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32: {
        auto val = key.GetInt32Value();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_INT64: {
        auto val = key.GetInt64Value();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT32: {
        auto val = key.GetUInt32Value();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT64: {
        auto val = key.GetUInt64Value();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_BOOL: {
        auto val = key.GetBoolValue();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_STRING: {
        auto val = key.GetStringValue();
        RedisModule_ReplyWithStringBuffer(ctx, val.data(), val.size());
        break;
    }
    default:
        assert(false);
    }

    // Reply with value.
    switch (field.map_value_type()) {
    case gp::FieldDescriptor::CPPTYPE_INT32: {
        auto val = value.GetInt32Value();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_INT64: {
        auto val = value.GetInt64Value();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT32: {
        auto val = value.GetUInt32Value();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_UINT64: {
        auto val = value.GetUInt64Value();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_DOUBLE: {
        auto val = value.GetDoubleValue();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_FLOAT: {
        auto val = value.GetFloatValue();
        auto str = std::to_string(val);
        RedisModule_ReplyWithSimpleString(ctx, str.data());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_BOOL: {
        auto val = value.GetBoolValue();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_ENUM: {
        auto val = value.GetEnumValue();
        RedisModule_ReplyWithLongLong(ctx, val);
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_STRING: {
        auto val = value.GetStringValue();
        RedisModule_ReplyWithStringBuffer(ctx, val.data(), val.size());
        break;
    }
    case gp::FieldDescriptor::CPPTYPE_MESSAGE: {
        const auto &msg = value.GetMessageValue();
        _get_msg(ctx, msg, format);
        break;
    }
    default:
        assert(false);
    }
}

void GetCommand::_reply_with_nil(RedisModuleCtx *ctx) const {
    RedisModule_ReplyWithNull(ctx);
}

void GetCommand::_reply_with_msg(RedisModuleCtx *ctx,
        gp::Message &msg,
        const Args &args) const {
    const auto &path = args.path;
    if (msg.GetTypeName() != path.type()) {
        throw Error("type mismatch");
    }

    if (path.empty()) {
        // Get the whole message.
        return _get_msg(ctx, msg, args.format);
    }

    // Get field.
    _get_field(ctx, ConstFieldRef(&msg, path), args.format);
}

}

}

}
