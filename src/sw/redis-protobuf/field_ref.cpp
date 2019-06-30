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

#include "field_ref.h"
#include <google/protobuf/util/json_util.h>
#include "redis_protobuf.h"

namespace sw {

namespace redis {

namespace pb {

Path::Path(const StringView &str) {
    const auto *ptr = str.data();
    assert(ptr != nullptr);

    auto len = str.size();

    std::size_t type_len = 0;
    std::tie(_type, type_len) = _parse_type(ptr, len);

    if (type_len < len) {
        // Has fields.
        _fields = _parse_fields(ptr + type_len, len - type_len);
    }
}

std::pair<std::string, std::size_t> Path::_parse_type(const char *ptr, std::size_t len) {
    assert(ptr != nullptr);

    if (len == 0) {
        throw Error("empty type");
    }

    std::size_t idx = 0;
    for (; idx != len; ++idx) {
        // e.g. name::space::type.field1.field2
        if (ptr[idx] == '.') {
            break;
        }
    }

    // name::space::type => name.space.type
    std::string type;
    type.reserve(idx);
    for (std::size_t i = 0; i != idx; ++i) {
        auto ch = ptr[i];
        if (ch != ':') {
            type.push_back(ch);
        } else if (!type.empty() && type.back() != '.') {
            type.push_back('.');
        } // else discard duplicate ':'
    }

    return {std::move(type), idx};
}

std::vector<std::string> Path::_parse_fields(const char *ptr, std::size_t len) {
    assert(ptr != nullptr && len > 0 && ptr[0] == '.');

    std::vector<std::string> fields;
    std::size_t start = 1;
    for (std::size_t idx = 1; idx != len; ++idx) {
        if (ptr[idx] == '.') {
            if (idx <= start) {
                throw Error("empty field");
            }

            fields.emplace_back(ptr + start, idx - start);
            start = idx + 1;
        }
    }

    if (len <= start) {
        throw Error("empty field");
    }

    fields.emplace_back(ptr + start, len - start);

    return fields;
}

}

}

}
