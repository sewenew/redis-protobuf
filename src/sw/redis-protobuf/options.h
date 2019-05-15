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

#ifndef SEWENEW_REDISPROTOBUF_OPTIONS_H
#define SEWENEW_REDISPROTOBUF_OPTIONS_H

#include "module_api.h"
#include <string>

namespace sw {

namespace redis {

namespace pb {

struct Options {
    static Options& instance();

    void load(RedisModuleString **argv, int argc);

    std::string proto_dir;

private:
    Options() = default;

    Options(const Options &) = default;
    Options& operator=(const Options &) = default;

    Options(Options &&) = default;
    Options& operator=(Options &&) = default;

    ~Options() = default;
};

}

}

}

#endif // end SEWENEW_REDISPROTOBUF_OPTIONS_H
