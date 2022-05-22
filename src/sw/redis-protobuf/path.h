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

#ifndef SEWENEW_REDISPROTOBUF_PATH_H
#define SEWENEW_REDISPROTOBUF_PATH_H

#include <vector>
#include <string>
#include "utils.h"

namespace sw {
    
namespace redis {
    
namespace pb {

class Path {
public:
    Path() = default;

    Path(const StringView &type, const StringView &path);

    explicit Path(const StringView &type) : _type(type.data(), type.size()) {}

    const std::string& type() const {
        return _type;
    }

    const std::vector<std::string>& fields() const {
        return _fields;
    }

    bool empty() const {
        return _fields.empty();
    }

private:
    std::vector<std::string> _parse_fields(const StringView &path) const;

    std::string _type;

    std::vector<std::string> _fields;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_PATH_H
