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

#include "path.h"
#include "errors.h"

namespace sw {
    
namespace redis {
    
namespace pb {

Path::Path(const StringView &type, const StringView &path) :
    _type(type.data(), type.size()), _fields(_parse_fields(path)) {}

std::vector<std::string> Path::_parse_fields(const StringView &path) const {
    if (path.size() <= 1) {
        throw Error("empty path");
    }

    if (*(path.data()) != '/') {
        throw Error("invalid path: should begin with /");
    }

    std::vector<std::string> fields;
    auto start = 1U;
    const auto *ptr = path.data();
    for (auto idx = start; idx != path.size(); ++idx) {
        if (ptr[idx] == '/') {
            if (idx <= start) {
                throw Error("empty field");
            }

            fields.emplace_back(ptr + start, idx - start);
            start = idx + 1;
        }
    }

    if (path.size() <= start) {
        throw Error("empty field");
    }

    fields.emplace_back(ptr + start, path.size() - start);

    return fields;
}

}

}

}
