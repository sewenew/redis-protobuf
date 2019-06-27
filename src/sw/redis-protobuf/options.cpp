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

#include "options.h"
#include <vector>
#include "redis_protobuf.h"
#include "errors.h"
#include "utils.h"

namespace sw {

namespace redis {

namespace pb {

void Options::load(RedisModuleString **argv, int argc) {
    Options opts;

    auto idx = 0;
    while (idx < argc) {
        auto opt = StringView(argv[idx]);

        if (util::str_case_equal(opt, "--DIR")) {
            if (!opts.proto_dir.empty()) {
                throw Error("duplicate --DIR option");
            }

            ++idx;

            opts.proto_dir = util::sv_to_string(StringView(argv[idx]));
        } else {
            throw Error("unknown option: " + util::sv_to_string(opt));
        }

        ++idx;
    }

    if (opts.proto_dir.empty()) {
        throw Error("option '--DIR dir' is required");
    }

    *this = std::move(opts);
}

}

}

}
