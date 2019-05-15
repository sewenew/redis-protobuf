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
#include <unistd.h>
#include <vector>
#include "redis_protobuf.h"
#include "errors.h"

namespace sw {

namespace redis {

namespace pb {

Options& Options::instance() {
    static Options opts;

    return opts;
}

void Options::load(RedisModuleString **argv, int argc) {
    std::vector<char*> argvs;
    argvs.reserve(argc + 1);
    argvs.push_back(const_cast<char*>(MODULE_NAME));
    for (auto idx = 0; idx != argc; ++idx) {
        argvs.push_back(const_cast<char*>(RedisModule_StringPtrLen(argv[idx], nullptr)));
    }

    Options opts;
    int opt = 0;
    while ((opt = getopt(argc + 1, argvs.data(), "d:")) != -1) {
        try {
            switch (opt) {
            case 'd':
                opts.proto_dir = optarg;
                break;

            default:
                throw Error("Unknown command line option");
            }
        } catch (const Error &e) {
            throw;
        }
    }

    if (opts.proto_dir.empty()) {
        throw Error("proto directory is not specified with -p option");
    }

    *this = std::move(opts);
}

}

}

}
