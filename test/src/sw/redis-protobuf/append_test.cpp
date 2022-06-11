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

#include "append_test.h"
#include "utils.h"

namespace sw {

namespace redis {

namespace pb {

namespace test {

void AppendTest::run() {
    auto key = test_key("append");

    KeyDeleter deleter(_redis, key);

    REDIS_ASSERT(_redis.command<long long>("PB.SET", key, "Msg",
                R"({"i" : 1})") == 1,
            "failed to test pb.append command");

    REDIS_ASSERT(_redis.command<long long>("PB.APPEND", key, "Msg",
                "/sub/s", "abc") == 3,
            "failed to test appending string");

    REDIS_ASSERT(_redis.command<long long>("PB.APPEND", key, "Msg",
                "/sub/s", "123") == 6,
            "failed to test appending string");

    REDIS_ASSERT(_redis.command<std::string>("PB.GET", key, "Msg",
                "/sub/s") == "abc123",
            "failed to test pb.append");

    REDIS_ASSERT(_redis.command<long long>("PB.APPEND", key, "Msg",
                "/arr", 1, 2) == 2,
            "failed to test appending array");

    REDIS_ASSERT(_redis.command<long long>("PB.APPEND", key, "Msg",
                "/arr", 3, 4) == 4,
            "failed to test appending array");

    REDIS_ASSERT(_redis.command<long long>("PB.GET", key, "Msg",
                "/arr/0") == 1,
            "failed to test pb.append");
    try {
        _redis.command<long long>("PB.APPEND", key, "Msg",
                "/i", 2);
        REDIS_ASSERT(false, "failed to test pb.append");
    } catch (const sw::redis::Error &) {
    }
}

}

}

}

}
