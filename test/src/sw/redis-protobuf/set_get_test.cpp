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

#include "set_get_test.h"
#include "utils.h"
#include <iostream>

namespace sw {

namespace redis {

namespace pb {

namespace test {

void SetGetTest::_run(sw::redis::Redis &r) {
    auto key = test_key("set-get");

    //KeyDeleter deleter(r, key);
    r.command("PB.SET", "key", "Msg", R"({"i" : 1})");

    REDIS_ASSERT(r.command<long long>("PB.SET", key, "Msg",
                R"({"i" : 1})") == 1 &&
                r.command<long long>("PB.GET", key, "Msg", "/i") == 1,
            "failed to test pb.set and pb.get command");

    REDIS_ASSERT(r.command<long long>("PB.SET", key, "Msg",
                "/i", 2) == 1 &&
                r.command<long long>("PB.GET", key, "Msg", "/i") == 2,
            "failed to test pb.set and pb.get command");

    REDIS_ASSERT(r.command<long long>("PB.SET", key, "Msg",
                R"({"sub" : {"s" : "hello"}})") == 1 &&
                r.command<std::string>("PB.GET", key, "Msg", "/sub/s") == "hello",
            "failed to test pb.set and pb.get command");

    REDIS_ASSERT(r.command<long long>("PB.SET", key, "Msg",
                "/sub/s", "world") == 1 &&
                r.command<std::string>("PB.GET", key, "Msg", "/sub/s") == "world",
            "failed to test pb.set and pb.get command");

    REDIS_ASSERT(r.command<long long>("PB.SET", key, "Msg",
                R"({"i" : 123, "sub" : {"s" : "hello", "i" : 123}, "arr" : [1, 2], "m" : {"k" : "v"}})") == 1 &&
                r.command<std::string>("PB.GET", key, "Msg", "/sub/s") == "hello" &&
                r.command<long long>("PB.GET", key, "Msg", "/arr/0") == 1,
            "failed to test pb.set and pb.get command");

    REDIS_ASSERT(r.command<long long>("PB.SET", key, "Msg",
                "/m/key", "world") == 1 &&
                r.command<std::string>("PB.GET", key, "Msg", "/m/key") == "world",
            "failed to test pb.set and pb.get command");

    REDIS_ASSERT(r.command<long long>("PB.SET", key, "Msg",
                R"({"arr" : [4, 5, 6]})") == 1,
            "failed to test pb.set command");

    auto arr = r.command<std::vector<long long>>("PB.GET", key, "Msg", "/arr");
    auto tmp = std::vector<long long>{4, 5, 6};
    REDIS_ASSERT(arr == tmp, "failed to test pb.get command");

    REDIS_ASSERT(r.command<long long>("PB.SET", key, "--NX", "Msg",
                "/sub/s", "world") == 0,
            "failed to test pb.set and pb.get command");
}

}

}

}

}
