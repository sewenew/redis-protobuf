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

#include "clear_test.h"
#include "utils.h"

namespace sw {

namespace redis {

namespace pb {

namespace test {

void ClearTest::run() {
    auto key = test_key("clear");

    KeyDeleter deleter(_redis, key);

    REDIS_ASSERT(_redis.command<long long>("PB.SET", key, "Msg",
                R"({"i" : 1, "sub" : {"s" : "hello"}, "arr" : [1, 2]})") == 1,
            "failed to test pb.append command");

    REDIS_ASSERT(_redis.command<long long>("PB.CLEAR ", key, "Msg") == 1 &&
                _redis.command<long long>("PB.GET", key, "Msg", "/i") == 0,
            "failed to test clear whole message");

    REDIS_ASSERT(_redis.command<long long>("PB.SET", key, "Msg",
                R"({"i" : 1, "sub" : {"s" : "hello", "i" : 23}, "arr" : [1, 2]})") == 1,
            "failed to test pb.clear command");

    REDIS_ASSERT(_redis.command<long long>("PB.CLEAR", key, "Msg", "/i") == 1 &&
                _redis.command<long long>("PB.GET", key, "Msg", "/i") == 0,
            "failed to test clear int");

    REDIS_ASSERT(_redis.command<long long>("PB.CLEAR", key, "Msg", "/sub/s") == 1 &&
                _redis.command<std::string>("PB.GET", key, "Msg", "/sub/s").empty(),
            "failed to test clear string");

    REDIS_ASSERT(_redis.command<long long>("PB.CLEAR", key, "Msg", "/arr") == 1 &&
                _redis.command<long long>("PB.LEN", key, "Msg", "/arr") == 0,
            "failed to test clear array");

    REDIS_ASSERT(_redis.command<long long>("PB.CLEAR", key, "Msg", "/sub") == 1 &&
                _redis.command<long long>("PB.GET", key, "Msg", "/sub/i") == 0,
            "failed to test clear sub message");
}

}

}

}

}
