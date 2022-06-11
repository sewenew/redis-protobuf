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

#include "len_test.h"
#include "utils.h"

namespace sw {

namespace redis {

namespace pb {

namespace test {

void LenTest::run() {
    auto key = test_key("len");

    KeyDeleter deleter(_redis, key);

    REDIS_ASSERT(_redis.command<long long>("PB.SET", key, "Msg",
                R"({"sub" : {"s" : "hello"}, "arr" : [1, 2, 3], "m" : {"k1" : "v1", "k2" : "v2"}})") == 1,
            "failed to test pb.len command");

    REDIS_ASSERT(_redis.command<long long>("PB.LEN", key, "Msg",
                "/sub/s") == 5,
            "failed to test pb.len with string");

    REDIS_ASSERT(_redis.command<long long>("PB.LEN", key, "Msg",
                "/arr") == 3,
            "failed to test pb.len with array");

    REDIS_ASSERT(_redis.command<long long>("PB.LEN", key, "Msg",
                "/m") == 2,
            "failed to test pb.len with map");
}

}

}

}

}
